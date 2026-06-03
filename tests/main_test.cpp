#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "main.h"

// ---- CompactSize ------------------------------------------------------------

namespace {

void expect_roundtrip(uint64_t value, std::size_t expected_size) {
    CompactSizeEncoder encoder;
    CompactSizeDecoder decoder;

    auto encoded = encoder.encode(value);
    ASSERT_EQ(encoded.size(), expected_size) << "value = " << value;

    auto decoded = decoder.decode(encoded);
    ASSERT_TRUE(decoded.has_value()) << "value = " << value;
    EXPECT_EQ(decoded->first, value);
    EXPECT_EQ(decoded->second, expected_size);
}

}  // namespace

TEST(CompactSize, SingleByte) {
    expect_roundtrip(0, 1);
    expect_roundtrip(1, 1);
    expect_roundtrip(127, 1);
    expect_roundtrip(252, 1);
}

TEST(CompactSize, TwoBytePrefix) {
    expect_roundtrip(253, 3);
    expect_roundtrip(254, 3);
    expect_roundtrip(65535, 3);
}

TEST(CompactSize, FourBytePrefix) {
    expect_roundtrip(65536, 5);
    expect_roundtrip(65537, 5);
    expect_roundtrip(4294967295u, 5);
}

TEST(CompactSize, EightBytePrefix) {
    expect_roundtrip(4294967296ull, 9);
    expect_roundtrip(std::numeric_limits<uint64_t>::max(), 9);
}

TEST(CompactSize, EncodingByteLayout) {
    CompactSizeEncoder encoder;

    EXPECT_EQ(encoder.encode(0), (std::vector<uint8_t>{0x00}));
    EXPECT_EQ(encoder.encode(252), (std::vector<uint8_t>{0xfc}));
    EXPECT_EQ(encoder.encode(253), (std::vector<uint8_t>{0xfd, 0xfd, 0x00}));
    EXPECT_EQ(encoder.encode(65536),
              (std::vector<uint8_t>{0xfe, 0x00, 0x00, 0x01, 0x00}));
    EXPECT_EQ(encoder.encode(4294967296ull),
              (std::vector<uint8_t>{0xff, 0x00, 0x00, 0x00, 0x00,
                                    0x01, 0x00, 0x00, 0x00}));
}

TEST(CompactSize, DecodeEmpty) {
    CompactSizeDecoder decoder;
    EXPECT_FALSE(decoder.decode({}).has_value());
}

TEST(CompactSize, DecodeTruncated) {
    CompactSizeDecoder decoder;
    EXPECT_FALSE(decoder.decode({0xfd, 0x01}).has_value());        // needs 3 bytes
    EXPECT_FALSE(decoder.decode({0xfe, 0x01, 0x02}).has_value());  // needs 5 bytes
    EXPECT_FALSE(decoder.decode({0xff, 0x01, 0x02, 0x03}).has_value());  // needs 9 bytes
}

// ---- TransactionData --------------------------------------------------------

TEST(TransactionData, AddInputDefaultsAndShape) {
    TransactionData tx;
    tx.add_input("abc", 0, "sig1");
    tx.add_input("def", 1, "sig2", 0x12345678u);

    ASSERT_EQ(tx.inputs.size(), 2u);
    EXPECT_EQ(tx.inputs[0].prev_txid, "abc");
    EXPECT_EQ(tx.inputs[0].prev_vout, 0u);
    EXPECT_EQ(tx.inputs[0].script_sig, "sig1");
    EXPECT_EQ(tx.inputs[0].sequence, 0xFFFFFFFFu);
    EXPECT_EQ(tx.inputs[1].sequence, 0x12345678u);
}

TEST(TransactionData, AddOutput) {
    TransactionData tx;
    tx.add_output(500, "script_a");
    tx.add_output(1500, "script_b");

    ASSERT_EQ(tx.outputs.size(), 2u);
    EXPECT_EQ(tx.outputs[0].first, 500u);
    EXPECT_EQ(tx.outputs[0].second, "script_a");
    EXPECT_EQ(tx.outputs[1].first, 1500u);
    EXPECT_EQ(tx.outputs[1].second, "script_b");
}

TEST(TransactionData, GetInputDetailsCopies) {
    TransactionData tx;
    tx.add_input("abc", 0, "sig1");
    tx.add_input("def", 1, "sig2");

    auto details = tx.get_input_details();
    ASSERT_EQ(details.size(), 2u);
    EXPECT_EQ(details[0].prev_txid, "abc");
    EXPECT_EQ(details[1].prev_txid, "def");

    // Mutating the returned copy must not affect the original.
    details[0].prev_txid = "mutated";
    EXPECT_EQ(tx.inputs[0].prev_txid, "abc");
}

TEST(TransactionData, SummarizeOutputsBasic) {
    TransactionData tx;
    tx.add_output(100, "a");
    tx.add_output(200, "b");
    tx.add_output(50, "c");

    auto [total, count] = tx.summarize_outputs();
    EXPECT_EQ(total, 350u);
    EXPECT_EQ(count, 3u);
}

TEST(TransactionData, SummarizeOutputsSkipsBelowMin) {
    TransactionData tx;
    tx.add_output(100, "a");
    tx.add_output(200, "b");
    tx.add_output(50, "c");
    tx.add_output(300, "d");

    auto [total, count] = tx.summarize_outputs(150);
    EXPECT_EQ(total, 500u);  // 200 + 300
    EXPECT_EQ(count, 2u);
}

TEST(TransactionData, SummarizeOutputsBreaksOverBillion) {
    TransactionData tx;
    tx.add_output(600'000'000ull, "a");
    tx.add_output(600'000'000ull, "b");  // running total crosses 1B here -> break
    tx.add_output(100ull, "c");          // must not be counted
    tx.add_output(200ull, "d");

    auto [total, count] = tx.summarize_outputs();
    EXPECT_EQ(total, 1'200'000'000ull);
    EXPECT_EQ(count, 2u);
}

TEST(TransactionData, UpdateMetadataMerges) {
    TransactionData tx;
    tx.update_metadata({{"network", "mainnet"}, {"size", "250"}});
    tx.update_metadata({{"size", "275"}, {"fee_rate", "10"}});  // overwrite + add

    EXPECT_EQ(tx.metadata.size(), 3u);
    EXPECT_EQ(tx.metadata.at("network"), "mainnet");
    EXPECT_EQ(tx.metadata.at("size"), "275");
    EXPECT_EQ(tx.metadata.at("fee_rate"), "10");
}

TEST(TransactionData, GetMetadataValueWithDefault) {
    TransactionData tx;
    tx.update_metadata({{"network", "mainnet"}});

    EXPECT_EQ(tx.get_metadata_value("network"), "mainnet");
    EXPECT_EQ(tx.get_metadata_value("missing"), "");
    EXPECT_EQ(tx.get_metadata_value("missing", "fallback"), "fallback");
}

TEST(TransactionData, GetTransactionHeader) {
    TransactionData tx(2, 500);
    tx.add_input("abc", 0, "sig");
    tx.add_output(100, "script");
    tx.add_output(200, "script");

    auto header = tx.get_transaction_header();
    EXPECT_EQ(std::get<0>(header), 2u);
    EXPECT_EQ(std::get<1>(header), 1u);
    EXPECT_EQ(std::get<2>(header), 2u);
    EXPECT_EQ(std::get<3>(header), 500u);
}

TEST(TransactionData, SetTransactionHeader) {
    TransactionData tx;
    tx.set_transaction_header(7, 99, 99, 1234);
    EXPECT_EQ(tx.version, 7u);
    EXPECT_EQ(tx.lock_time, 1234u);
    // num_inputs / num_outputs arguments are intentionally ignored.
    EXPECT_EQ(tx.inputs.size(), 0u);
    EXPECT_EQ(tx.outputs.size(), 0u);
}

// ---- UTXOSet ----------------------------------------------------------------

TEST(UTXOSet, AddAndRemove) {
    UTXOSet set;
    set.add_utxo("txid1", 0, 1000);
    set.add_utxo("txid2", 1, 2000);
    EXPECT_EQ(set.utxos.size(), 2u);

    EXPECT_TRUE(set.remove_utxo("txid1", 0, 1000));
    EXPECT_EQ(set.utxos.size(), 1u);
    EXPECT_FALSE(set.remove_utxo("txid1", 0, 1000));  // already gone
    EXPECT_FALSE(set.remove_utxo("txid9", 5, 99));    // never present
}

TEST(UTXOSet, Balance) {
    UTXOSet set;
    EXPECT_EQ(set.get_balance(), 0u);
    set.add_utxo("a", 0, 100);
    set.add_utxo("b", 0, 250);
    set.add_utxo("c", 0, 750);
    EXPECT_EQ(set.get_balance(), 1100u);
}

TEST(UTXOSet, FindSufficientUtxos) {
    UTXOSet set;
    set.add_utxo("a", 0, 100);
    set.add_utxo("b", 0, 250);
    set.add_utxo("c", 0, 750);

    auto picked = set.find_sufficient_utxos(300);
    uint64_t total = 0;
    for (const auto& u : picked) total += u.amount;
    EXPECT_GE(total, 300u);
    EXPECT_FALSE(picked.empty());

    // Target larger than the entire balance returns an empty set.
    auto impossible = set.find_sufficient_utxos(10'000);
    EXPECT_TRUE(impossible.empty());
}

TEST(UTXOSet, Count) {
    UTXOSet set;
    EXPECT_EQ(set.get_total_utxo_count(), 0u);
    set.add_utxo("a", 0, 1);
    set.add_utxo("b", 0, 2);
    EXPECT_EQ(set.get_total_utxo_count(), 2u);
}

TEST(UTXOSet, IsSubsetOf) {
    UTXOSet small;
    small.add_utxo("a", 0, 1);
    small.add_utxo("b", 0, 2);

    UTXOSet big;
    big.add_utxo("a", 0, 1);
    big.add_utxo("b", 0, 2);
    big.add_utxo("c", 0, 3);

    EXPECT_TRUE(small.is_subset_of(big));
    EXPECT_FALSE(big.is_subset_of(small));
    EXPECT_TRUE(small.is_subset_of(small));
}

TEST(UTXOSet, CombineUtxosUnion) {
    UTXOSet a;
    a.add_utxo("x", 0, 10);
    a.add_utxo("y", 0, 20);

    UTXOSet b;
    b.add_utxo("y", 0, 20);  // shared
    b.add_utxo("z", 0, 30);

    auto combined = a.combine_utxos(b);
    EXPECT_EQ(combined.utxos.size(), 3u);
    EXPECT_EQ(combined.get_balance(), 60u);
}

TEST(UTXOSet, FindCommonUtxosIntersection) {
    UTXOSet a;
    a.add_utxo("x", 0, 10);
    a.add_utxo("y", 0, 20);
    a.add_utxo("z", 0, 30);

    UTXOSet b;
    b.add_utxo("y", 0, 20);
    b.add_utxo("z", 0, 30);
    b.add_utxo("w", 0, 99);

    auto common = a.find_common_utxos(b);
    EXPECT_EQ(common.utxos.size(), 2u);
    EXPECT_EQ(common.get_balance(), 50u);
}

// ---- Block header generator -------------------------------------------------

TEST(BlockHeader, CallbackFiresMaxAttemptsTimes) {
    int count = 0;
    generate_block_headers(
        "prev", "merkle", 1700000000u, 0x1d00ffffu, 0, 5,
        [&](const BlockHeader&) {
            ++count;
            return true;
        });
    EXPECT_EQ(count, 5);
}

TEST(BlockHeader, NonceIncrementsFromStart) {
    std::vector<uint32_t> seen_nonces;
    generate_block_headers(
        "prev", "merkle", 1700000000u, 0x1d00ffffu, 100, 4,
        [&](const BlockHeader& h) {
            seen_nonces.push_back(h.nonce);
            return true;
        });
    ASSERT_EQ(seen_nonces.size(), 4u);
    EXPECT_EQ(seen_nonces[0], 100u);
    EXPECT_EQ(seen_nonces[1], 101u);
    EXPECT_EQ(seen_nonces[2], 102u);
    EXPECT_EQ(seen_nonces[3], 103u);
}

TEST(BlockHeader, CallbackCanStopEarly) {
    int count = 0;
    generate_block_headers(
        "prev", "merkle", 1700000000u, 0x1d00ffffu, 0, 1000,
        [&](const BlockHeader&) {
            ++count;
            return count < 3;  // stop after the 3rd call
        });
    EXPECT_EQ(count, 3);
}

TEST(BlockHeader, HeaderFieldsArePopulated) {
    BlockHeader captured;
    generate_block_headers(
        "prev_hash_xyz", "merkle_root_abc", 1700000000u, 0x1d00ffffu, 42, 1,
        [&](const BlockHeader& h) {
            captured = h;
            return false;
        });
    EXPECT_EQ(captured.prev_block_hash, "prev_hash_xyz");
    EXPECT_EQ(captured.merkle_root, "merkle_root_abc");
    EXPECT_EQ(captured.timestamp, 1700000000u);
    EXPECT_EQ(captured.bits, 0x1d00ffffu);
    EXPECT_EQ(captured.nonce, 42u);
}
