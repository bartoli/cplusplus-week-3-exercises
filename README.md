# Btrust Builders: C++ for Bitcoiners Week Three Exercises

## Instructions

Welcome! This exercise is designed to help you practice key C++ programming skills specifically for Bitcoin development. 
Your task is to complete the `// TODO:` items found in `src/main.cpp` and `src/main.h`.

1. Fork this repository.
2. Go to the `Actions` tab and enable github workflow for your repository by clicking `I understand my ...`

<img src="https://github.com/btrust-builders/cplusplus-week-3-exercises/blob/main/enable-github-actions.png" width="700" />

3. Clone your fork to your local computer.
4. **Explore the Code**
   - Open `src/main.cpp` and `src/main.h` and read through the function stubs.
   - Look for comments marked with `// TODO:` — those are the spots you need to implement.
   - Function signatures and supporting types are declared in `src/main.h`.
5. **Complete the TODOs**
   - Implement the missing logic where indicated.
   - Ensure your code is readable, idiomatic, and compiles without warnings.
6. **Build and Test Your Code**
   - Configure the build (only needed once, or after editing `CMakeLists.txt`):
     ```bash
     cmake -S . -B build
     ```
   - Build:
     ```bash
     cmake --build build
     ```
   - Run the tests:
     ```bash
     ./build/test_main
     ```
   - Or run via `ctest` for a summary view:
     ```bash
     ctest --test-dir build --output-on-failure
     ```
7. Commit and push your changes to the `main` branch of your remote fork.

8. Confirm your forked repository has a green check mark.

<img src="https://github.com/btrust-builders/cplusplus-week-3-exercises/blob/main/success.png" width="500" />

9. Submit your solution to this form: [Google form](https://forms.gle/Hr7beNMa6h4FdTgh9).

PS: You can commit and push as often as you like and GitHub Actions will re-evaluate your code every time.
You will need to look through the auto-grader logs (in the "Actions" tab) to see what exactly you got right or wrong.

## Resources
- Learn me a Bitcoin: [link](https://learnmeabitcoin.com/)

## Plagiarism Policy
Submissions must be your own work. Do not copy from other students or past cohorts. Sharing or copying answers is not allowed. Violations may result in your submission being invalidated or in follow-up under the program’s policies.

## AI Usage Disclaimer
You may use AI tools to study and to clarify concepts, but the code you submit must be written by you. Do not paste in AI-generated answers as your submission. The exercises are for your learning; relying on AI to produce answers will undermine that and may be treated like other academic misconduct.

## Why These Restrictions?
These rules are in place so that you actually practice and learn C++, build real skills, and can be fairly assessed. Doing the work yourself is how you learn; shortcuts defeat the purpose of the exercises and don’t prepare you for what comes next in the program.
