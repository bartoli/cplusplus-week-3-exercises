#include "main.h"
 #include <algorithm>
 #include <iterator>

// ---- Supporting type member functions ---------------------------------------

bool TxInput::operator==(const TxInput& other) const {
    return prev_txid == other.prev_txid
        && prev_vout == other.prev_vout
        && script_sig == other.script_sig
        && sequence == other.sequence;
}

// Provided so UTXO can be stored in std::set. Order by (tx_id, vout_index, amount).
bool UTXO::operator<(const UTXO& other) const {
    if (tx_id != other.tx_id) return tx_id < other.tx_id;
    if (vout_index != other.vout_index) return vout_index < other.vout_index;
    return amount < other.amount;
}

bool UTXO::operator==(const UTXO& other) const {
    return tx_id == other.tx_id
        && vout_index == other.vout_index
        && amount == other.amount;
}

// ---- Exercises --------------------------------------------------------------

std::vector<uint8_t> CompactSizeEncoder::encode(uint64_t value) {
    std::vector<uint8_t> out;
    if(value<0xFD)
    {
        out.push_back(value & 0xFF);
        return out;
    }
    if(value<=0xFFFF)
    {
        out.push_back(0xFD);
        out.push_back(value&0xFF);
        out.push_back((value>>8)&0xFF);        
        return out;
    }
    if(value<=0xFFFFFFFF)
    {
        out.push_back(0xFE);
        out.push_back(value&0xFF);
        out.push_back((value>>8)&0xFF);
        out.push_back((value>>16)&0xFF);
        out.push_back((value>>24)&0xFF);
        return out;
    }
    out.push_back(0xFF);
    out.push_back(value&0xFF);    
    out.push_back((value>>8)&0xFF);
    out.push_back((value>>16)&0xFF);
    out.push_back((value>>24)&0xFF);
    out.push_back((value>>32)&0xFF);
    out.push_back((value>>40)&0xFF);
    out.push_back((value>>48)&0xFF);
    out.push_back((value>>56)&0xFF);
    
    return out;
            
    // TODO: encode `value` into Bitcoin CompactSize bytes.
    //   value < 0xFD          -> single byte
    //   value <= 0xFFFF       -> 0xFD prefix + 2 little-endian bytes
    //   value <= 0xFFFFFFFF   -> 0xFE prefix + 4 little-endian bytes
    //   otherwise             -> 0xFF prefix + 8 little-endian bytes
    
}

std::optional<std::pair<uint64_t, std::size_t>> CompactSizeDecoder::decode(
    const std::vector<uint8_t>& data) {
    // TODO: decode a CompactSize integer from the front of `data`.
    // Return std::nullopt if `data` is empty or shorter than the prefix
    // demands. On success return {value, bytes_consumed}.
    if(data.empty())
      return std::nullopt;
    if(data.size()>=1 && data[0]<0xFD)
    {
        uint64_t value = data[0];
        return std::pair<uint64_t,size_t>(value,1);
    }
    if(data[0] == 0xFF && data.size()>=9)
    {
        uint64_t value = 
        (((uint64_t)data[8])<<56)+
        (((uint64_t)data[7])<<48)+ 
        (((uint64_t)data[6])<<40)+
        (((uint64_t)data[5])<<32)+
        (((uint64_t)data[4])<<24) +
        (data[3]<<16) +
        (data[2]<<8) +
        data[1];
        return std::pair<uint64_t,size_t>(value, 9);
    }
    if(data[0] == 0xFE && data.size()>=5)
    {
        uint64_t value = (((uint64_t)data[4])<<24) + (data[3]<<16) + (data[2]<<8) + data[1];
        return std::pair<uint64_t,size_t>(value,5);
    }
    if(data[0] == 0xFD && data.size()>=3)
    {
        uint64_t value = (data[2]<<8) + data[1];
        return std::pair<uint64_t,size_t>(value,3);
    }
    
    return std::nullopt;
}

// ---- TransactionData --------------------------------------------------------

TransactionData::TransactionData(uint32_t version, uint32_t lock_time)
    : version(version), lock_time(lock_time) {}

void TransactionData::add_input(const std::string& tx_id,
                                uint32_t vout_index,
                                const std::string& script_sig,
                                uint32_t sequence) {
    // TODO: build a TxInput from the arguments and append it to `inputs`.
    inputs.push_back({tx_id, vout_index, script_sig, sequence});
}

void TransactionData::add_output(uint64_t value_satoshi,
                                 const std::string& script_pubkey) {
    // TODO: append value_satoshi and script_pubkey to `outputs`.
    outputs.push_back(std::pair<uint64_t,std::string>(value_satoshi, script_pubkey));
}

std::vector<TxInput> TransactionData::get_input_details() const {
    // TODO: iterate over `inputs` and return a copy of each entry.
    return inputs;
}

std::pair<uint64_t, std::size_t> TransactionData::summarize_outputs(
    uint64_t min_value) const {
        uint64_t sats = 0;
        int valid_outputs = 0;
        for(const auto& out : outputs)
        {
            if(out.first<min_value)
                continue;
            sats += out.first;
            valid_outputs++;
            if(sats>=1000000000LL)
              break;
        }
    // TODO: walk `outputs` with a while loop, summing values that are >= min_value.
    //       Skip (continue) outputs below min_value.
    //       Break out of the loop as soon as the running total exceeds
    //       1,000,000,000 satoshis. Return {total_satoshi, valid_outputs_count}.
    return {sats, valid_outputs};
}

void TransactionData::update_metadata(
    const std::unordered_map<std::string, std::string>& new_data) {
    // TODO: merge `new_data` into `metadata`, overwriting existing keys.
    auto it = new_data.begin();
    while(it != new_data.end())
    {
        metadata[it->first] = it->second;
        ++it;
    }    
}

std::string TransactionData::get_metadata_value(
    const std::string& key, const std::string& default_value) const {
    // TODO: return metadata[key] if present, otherwise `default_value`.
    auto it = metadata.find(key);
    if(it == metadata.end())
        return default_value;
    return it->second;
}

std::tuple<uint32_t, std::size_t, std::size_t, uint32_t>
TransactionData::get_transaction_header() const {
    // TODO: return the version, inputs.size(), outputs.size(), and lock_time.
    return {version, inputs.size(), outputs.size(), lock_time};
}

void TransactionData::set_transaction_header(uint32_t new_version,
                                             std::size_t num_inputs,
                                             std::size_t num_outputs,
                                             uint32_t new_lock_time) {
    // TODO: update `version` and `lock_time`. Ignore num_inputs/num_outputs.
    //       Hint: practice structured bindings or std::tie with `_` placeholders.
    version = new_version;
    (void)num_inputs;
    (void)num_outputs;
    lock_time = new_lock_time;
}

// ---- UTXOSet ----------------------------------------------------------------

void UTXOSet::add_utxo(const std::string& tx_id,
                       uint32_t vout_index,
                       uint64_t amount) {
    utxos.insert({tx_id, vout_index, amount});
}

bool UTXOSet::remove_utxo(const std::string& tx_id,
                          uint32_t vout_index,
                          uint64_t amount) {
    auto it = utxos.find({tx_id, vout_index, amount});
    if(it == utxos.end())
      return false;
    utxos.erase(it);
    return true;
}

uint64_t UTXOSet::get_balance() const {
    uint64_t sum = 0;
    for(const auto & utxo : utxos)
    {
        sum += utxo.amount;
    }
    return sum;
}

std::set<UTXO> UTXOSet::find_sufficient_utxos(uint64_t target_amount) const {
    std::set<UTXO> out;
    uint64_t sum = 0;
    for(const auto & utxo : utxos)
    {   
        out.insert(utxo);
        sum += utxo.amount;
        if(sum >= target_amount)
          break;
    }
    //did we have enough?
    if(sum<target_amount)
        return {};
    return out;
}

std::size_t UTXOSet::get_total_utxo_count() const {
    return utxos.size();
}

bool UTXOSet::is_subset_of(const UTXOSet& other) const {
    for(const auto& utxo : utxos)
    {
        if( other.utxos.find(utxo)== other.utxos.end())
          return false;
    }
    return true;
}

UTXOSet UTXOSet::combine_utxos(const UTXOSet& other) const {
    // TODO: return a new UTXOSet whose `utxos` is the union of this set's UTXOs and `other.utxos`.
    //       Hint: std::set_union.
    UTXOSet out;
    std::set_union(utxos.begin(), utxos.end(),
        other.utxos.begin(), other.utxos.end(),
        std::inserter(out.utxos, out.utxos.begin()));
    return out;
}

UTXOSet UTXOSet::find_common_utxos(const UTXOSet& other) const {
    // TODO: return a new UTXOSet whose `utxos` is the intersection of this set's UTXOs and `other.utxos`. 
    //       Hint: std::set_intersection.
    UTXOSet out;
    std::set_intersection(utxos.begin(), utxos.end(),
        other.utxos.begin(), other.utxos.end()
        , std::inserter(out.utxos, out.utxos.begin()));
    return out;
}

// ---- Block header generator -------------------------------------------------

void generate_block_headers(
    const std::string& prev_block_hash,
    const std::string& merkle_root,
    uint32_t timestamp,
    uint32_t bits,
    uint32_t start_nonce,
    uint32_t max_attempts,
    const std::function<bool(const BlockHeader&)>& on_header) {
    // TODO: starting from `start_nonce`, build a BlockHeader for each nonce and call on_header(header). Stop early if on_header returns false.
    //       Otherwise stop after `max_attempts` attempts.
    uint32_t nonce(start_nonce);
    uint32_t attempts = 0;
    bool done = false;
    while(!done)
    {
        BlockHeader bh{2, prev_block_hash, merkle_root,timestamp,bits, nonce};
        if(!on_header(bh))
          break;

        nonce++;
        attempts++;
        done = attempts>= max_attempts;
    }


    (void)prev_block_hash;
    (void)merkle_root;
    (void)timestamp;
    (void)bits;
    (void)start_nonce;
    (void)max_attempts;
    (void)on_header;
}
