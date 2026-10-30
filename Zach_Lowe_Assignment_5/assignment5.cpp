/*
 * EECS 348 Assignment 5
 * Program Name : Assignment5.cpp
 * Description  : Object-oriented, recursive brute-force Sudoku solver. For each
 *                puzzle file it prints the file name, the original puzzle, and
 *                EVERY solution found (or "No solution found").
 * Inputs       : puzzle1.txt ... puzzle5.txt (9 rows of 9 tokens; digits 1-9,
 *                '_' for a blank cell), or file names given on the command line.
 * Output       : Console output: file name, puzzle, solution(s), solution count.
 * Collaborators: ChatGPT for the explanation of some parts of the original code
 *                and modifications to the main function comments by Yunjing Ma.
 *                Zach Lowe for Assignment 5 modifications, assisted by Claude AI
 * Author       : Yunjing Ma (Original Author), with Gemini
 *                Revised by Zach Lowe (Assignment 5 Modifications), with Claude AI
 * Created      : 10/22/2026   Last revised: 10/29/2026
 */

// Assignment 5 revision history
//   First revision: solutions are also written to solutionN.txt (N = puzzle number).
//     Each file holds every solution in the same format as puzzleN.txt (9 rows, '_' = blank)
//     followed by "Solutions found: n", or just "No solution found." if none exist.
//   Second revision (3g): final solution check, one shared rule-checking method,
//     named constants instead of the magic numbers 9 and 3, and header/comment cleanup.
//     Search order, solving algorithm and memory use are unchanged.
//   Inputs: puzzle1.txt - puzzle5.txt   Outputs: console text plus solution1.txt - solution5.txt
//   Revision collaborator: Claude (Anthropic) helped write the revision code marked
//     "A5" (first revision) and "3g" (second revision) below.
//
// Original Assignment 4 header
//   Platform: Gemini website engine
//   Author: Yunjing Ma (Assignment 4)
//   Collaboration: ChatGPT helped explain the meaning of some parts of the code. The comments
//     in the modify function and in main were written by Yunjing Ma. A small amount of the
//     code is original Gemini code.
//
// Description:
// This program reads five Sudoku puzzle files (puzzle1.txt through puzzle5.txt),
// validates the input data, and solves each puzzle using a recursive depth-first
// search algorithm with backtracking. It prints the input filename, the original
// puzzle, and all valid solutions found (or a message if no solution exists).
//

#include <iostream>
#include <fstream>
#include <string>

// 3g: named constants replace the magic numbers 9 and 3 used throughout Assignment 4 and the first revision
const int BOARD_SIZE = 9;     // the board is BOARD_SIZE x BOARD_SIZE cells
const int BOX_SIZE = 3;       // each sub-grid is BOX_SIZE x BOX_SIZE cells
const int EMPTY_CELL = 0;     // value stored in the board for a blank cell


// class SudokuSolver
// @brief Encapsulates a 9x9 Sudoku board, file loading logic, validation checks,
// and the recursive backtracking solving algorithm.
class SudokuSolver {
private:
    int board[BOARD_SIZE][BOARD_SIZE];  // grid representing the Sudoku puzzle (EMPTY_CELL denotes empty cells)
    std::string filename;               // The input file name associated with this puzzle instance
    std::string solutionFilename;       // A5: output file name, e.g. "solution1.txt" (built from the puzzle name)
    std::ofstream solutionOut;          // A5: stream used to write every solution found to solutionFilename

    // A5: writes the current board to the solution file in the same layout as the puzzle input files
    void writeBoardToFile() {
        for (int r = 0; r < BOARD_SIZE; ++r) {                  // go through each row
            for (int c = 0; c < BOARD_SIZE; ++c) {              // go through each column
                solutionOut << (board[r][c] == EMPTY_CELL ? '_' : char('0' + board[r][c])); // digit, or '_' if blank
                if (c < BOARD_SIZE - 1) solutionOut << ' ';     // single space between cells, none after the last
            }
            solutionOut << "\n";                                // end the row
        }
    }

    // 3g: the single rule-checking method. It replaces the separate isValid() and
    // validateInitialBoard() routines, which repeated the same row/column/box checks.
    // @brief Checks whether the value num at (row, col) conflicts with any OTHER cell in the
    //        same row, column or 3x3 sub-grid. The cell itself is skipped, so the method works
    //        both for a blank cell that is about to receive num (the search) and for a cell that
    //        already holds num (checking the initial clues or a finished board).
    // @param row Row index of the cell (0-8)
    // @param col Column index of the cell (0-8)
    // @param num Digit (1-9) being checked
    // @return true if num does not repeat in the cell's row, column or sub-grid
    bool isValid(int row, int col, int num) const {
        // Check row and column constraints
        for (int i = 0; i < BOARD_SIZE; ++i) {
            if (i != col && board[row][i] == num) return false;   // the same digit appears elsewhere in the row
            if (i != row && board[i][col] == num) return false;   // the same digit appears elsewhere in the column
        }

        // Check 3x3 sub-grid constraints.
        // Subtracting the remainder moves to the top-left cell of the sub-grid, which starts at
        // index 0, 3 or 6.
        int startRow = row - (row % BOX_SIZE);
        int startCol = col - (col % BOX_SIZE);
        for (int r = startRow; r < startRow + BOX_SIZE; ++r) {
            for (int c = startCol; c < startCol + BOX_SIZE; ++c) {
                // The same digit appears elsewhere in the sub-grid
                if ((r != row || c != col) && board[r][c] == num) {
                    return false;
                }
            }
        }

        return true;
    }

    // 3g: board-level check built on isValid(). It is used on the initial clues (blanks allowed)
    // and on every completed board before it is reported (no blanks allowed).
    // @param requireComplete if true, a blank cell also makes the board invalid
    // @return true if every filled cell follows the Sudoku rules (and, if required, no cell is blank)
    bool isBoardValid(bool requireComplete) const {
        for (int r = 0; r < BOARD_SIZE; ++r) {
            for (int c = 0; c < BOARD_SIZE; ++c) {
                int num = board[r][c];
                if (num == EMPTY_CELL) {
                    if (requireComplete) return false;   // a finished board cannot contain blanks
                    continue;                            // blanks cannot break a rule in the initial clues
                }
                if (!isValid(r, c, num)) return false;   // this digit repeats in its row, column or sub-grid
            }
        }
        return true;
    }

    // @brief Recursive helper function implementing DFS with backtracking to find all solutions.
    void solveRecursive(int row, int col, int &solutionCount) {
        // Base case: if row reaches BOARD_SIZE, the entire grid has been filled
        if (row == BOARD_SIZE) {
            // 3g: final solution check. Confirm the finished board really follows every rule before
            // it is counted, printed or saved. This should never fail, so it is reported as an
            // internal error and the board is discarded rather than shown as a solution.
            if (!isBoardValid(true)) {
                std::cerr << "Internal error: a completed board failed the final check and was discarded.\n";
                return;
            }
            solutionCount++;                                // one more solution
            std::cout << "Solution " << solutionCount << ":\n";
            printBoard();                                   // output the result
            if (solutionCount > 1) solutionOut << "\n";     // A5: blank line separates one solution from the previous one
            writeBoardToFile();                             // A5: save this solution to solutionN.txt
            return;
        }

        // Determine coordinates for the next cell
        int nextRow = (col == BOARD_SIZE - 1) ? row + 1 : row;   // at the end of a row, move to the next row
        int nextCol = (col == BOARD_SIZE - 1) ? 0 : col + 1;     // at the end of a row, return to column 0
        // If the current cell is already pre-filled, skip to the next cell
        if (board[row][col] != EMPTY_CELL) {
            solveRecursive(nextRow, nextCol, solutionCount);
            return;
        }

        // Try placing digits 1 through 9 in the empty cell
        for (int num = 1; num <= BOARD_SIZE; ++num) {
            if (isValid(row, col, num)) {
                board[row][col] = num;

                // Recurse to the next cell
                solveRecursive(nextRow, nextCol, solutionCount);

                board[row][col] = EMPTY_CELL;   // Backtrack if it fails and try another branch
            }
        }
    }

public:
    // @brief Constructor that initializes the Sudoku solver with a target filename.
    // @param fname The name of the input text file.
    SudokuSolver(const std::string &fname) : filename(fname) {
        // A5: "puzzleN.txt" -> "solutionN.txt" by swapping the "puzzle" prefix for "solution"
        size_t pos = fname.find("puzzle");
        solutionFilename = (pos == std::string::npos) ? "solution_" + fname
                                                      : fname.substr(0, pos) + "solution" + fname.substr(pos + 6);
        // Initialize the board with blanks in preparation for reading the file
        for (int i = 0; i < BOARD_SIZE; ++i) {
            for (int j = 0; j < BOARD_SIZE; ++j) {
                board[i][j] = EMPTY_CELL;
            }
        }
    }

    // @brief Loads and parses the Sudoku puzzle from the input file. This checks only the file
    //        format (row count, column count, characters); the Sudoku rules are checked in solve().
    // @return true if the file was successfully loaded and parsed, false on error.
    bool loadPuzzle() {
        std::ifstream file(filename);
        if (!file.is_open()) {   // report an error if the file can't be read
            std::cerr << "Error: Could not open file " << filename << "\n";
            return false;
        }

        int row = 0;
        std::string line;
        while (getline(file, line)) {
            // A5 fix: ignore empty lines and also lines holding only spaces/tabs/'\r'
            // (Assignment 4 counted those as extra rows)
            if (line.find_first_not_of(" \t\r\n") == std::string::npos) continue;

            if (row >= BOARD_SIZE) {   // more rows than the board has
                std::cerr << "Error: File " << filename << " contains more than " << BOARD_SIZE << " rows.\n";
                return false;
            }

            int col = 0;   // start from the first column
            for (char ch : line) {
                // Skip whitespace characters
                if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') continue;

                if (col >= BOARD_SIZE) {   // more columns than the board has
                    std::cerr << "Error: Row " << row + 1 << " in " << filename << " exceeds " << BOARD_SIZE << " columns.\n";
                    return false;
                }

                // Parse cell value: '_' or '0' or '.' represents a blank cell
                if (ch == '_' || ch == '0' || ch == '.') {
                    board[row][col] = EMPTY_CELL;
                } else if (ch >= '1' && ch <= '9') {   // load the digit from the original puzzle
                    board[row][col] = ch - '0';
                } else {   // any other character is an error
                    std::cerr << "Error: Invalid character '" << ch << "' in " << filename << ".\n";
                    return false;
                }
                col++;   // move to the next column
            }
            if (col != BOARD_SIZE) {   // the row is too short
                std::cerr << "Error: Row " << row + 1 << " in " << filename << " has " << col << " columns (expected " << BOARD_SIZE << ").\n";
                return false;
            }
            row++;   // move to the next row
        }
        file.close();
        if (row != BOARD_SIZE) {   // the file is too short
            std::cerr << "Error: File " << filename << " contains only " << row << " rows (expected " << BOARD_SIZE << ").\n";
            return false;
        }

        return true;
    }

    // @brief Prints the current state of the Sudoku board in a formatted grid.
    void printBoard() const {
        for (int r = 0; r < BOARD_SIZE; ++r) {
            if (r > 0 && r % BOX_SIZE == 0) {   // horizontal divider between sub-grid rows
                std::cout << "------+-------+------\n";
            }
            for (int c = 0; c < BOARD_SIZE; ++c) {
                if (c > 0 && c % BOX_SIZE == 0) {   // vertical divider between sub-grid columns
                    std::cout << "| ";
                }
                if (board[r][c] == EMPTY_CELL) {
                    std::cout << "_ ";             // blank cell
                } else {
                    std::cout << board[r][c] << " ";
                }
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }


    // @brief Executes the solver on the loaded puzzle and prints results.
    void solve() {
        std::cout << "========================================\n";
        std::cout << "Input Filename: " << filename << "\n";
        std::cout << "Original Puzzle:\n";
        printBoard();

        solutionOut.open(solutionFilename);               // A5: create/overwrite solutionN.txt
        if (!solutionOut.is_open()) {                     // A5: report if the output file cannot be created
            std::cerr << "Error: Could not open file " << solutionFilename << " for writing.\n";
        }

        int solutionCount = 0;   // number of solutions found
        // A5: a puzzle whose given digits already conflict has no solution, so the search is skipped.
        // 3g: this now uses the same rule check as the search, with blanks allowed.
        if (isBoardValid(false)) {
            solveRecursive(0, 0, solutionCount);
        }

        if (solutionCount == 0) {   // no solution found
            std::cout << "No solution found.\n";
            solutionOut << "No solution found.\n";                       // A5: file holds only this line
        } else {
            std::cout << "Total solutions found: " << solutionCount << "\n";
            solutionOut << "Solutions found: " << solutionCount << "\n"; // A5: required final line
        }
        solutionOut.close();                              // A5: flush and close the solution file
        std::cout << "========================================\n\n";
    }
};

/**
 * @brief Main execution function that processes puzzle1.txt through puzzle5.txt.
 */
int main() {
    // Array containing the names of the five required puzzle files
    std::string puzzleFiles[5] = {
        "puzzle1.txt",
        "puzzle2.txt",
        "puzzle3.txt",
        "puzzle4.txt",
        "puzzle5.txt"
    };

    // Iterate through each puzzle file and solve it
    for (int i = 0; i < 5; ++i) {
        SudokuSolver solver(puzzleFiles[i]);

        // Load the puzzle and check for reading/format errors before solving
        if (solver.loadPuzzle()) {
            solver.solve();
        } else {   // the puzzle file can't be read or is malformed
            std::cout << "Skipping " << puzzleFiles[i] << " due to input errors.\n\n";
        }
    }

    return 0;
}
