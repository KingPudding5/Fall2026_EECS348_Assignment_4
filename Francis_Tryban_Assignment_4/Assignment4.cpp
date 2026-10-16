// Program name: EECS 348 Assignment 4
// Brief description: Solves Sudoku puzzles using recursive brute-force
//                    backtracking and prints every solution it finds.
// Inputs: puzzle1.txt through puzzle5.txt by default, or filenames supplied
//         as command-line arguments. Each puzzle must have nine rows of nine
//         whitespace-separated cells. Digits 1-9 are clues, and _ is a blank.
//         Empty lines are allowed between or around the puzzle rows.
// Outputs: Each filename, its input grid, every solution with a number, and
//          the total number of solutions. Prints a message when a file cannot
//          be read or when a puzzle has no solution.
// Author: Francis Tryban
// Creation date: October 12, 2026
// Revision date: October 15, 2026
// Collaborators: OpenAI Codex (GPT-6 Luna) wrote the base code. Claude Opus 5.5
//                (Anthropic) helped with the first round of changes and wrote my test scripts.
#include <array>     // fixed-size arrays used for the Sudoku grid.
#include <fstream>   // file streams for opening puzzle files.
#include <iostream>  // std::cout for printing grids and messages.
#include <sstream>   // string stream for reading one row
#include <string>
#include <vector>    // Stores the list of filenames to process.

using Grid = std::array<std::array<int, 9>, 9>;

// keep one puzzle's grid, validation, search, and printing together here.
// A cell value of zero means that the cell is empty.
class Sudoku {
private: // private functions and data
    Grid cells{};                    // Initializes every cell to zero.
    std::string solutionCount = "0"; // Stores the count as decimal text.
    bool cluesClash = false; // Records whether the original clues conflict.

    // check the row, column, and 3-by-3 box before placing a value.
    // The target cell must be empty so it does not conflict with itself.
    bool allowed(int row, int column, int value) const { // const does not change the grid
        for (int i = 0; i < 9; ++i) { // Visits each position in the row and column.
            if (cells[row][i] == value || cells[i][column] == value) // Checks both.
                return false; // Rejects a value already in either location.
        }

        int blockRow = (row / 3) * 3; // Finds the box's first row index.
        int blockColumn = (column / 3) * 3; // Finds the box's first column index.
        for (int r = blockRow; r < blockRow + 3; ++r) { // Visits the box's rows.
            for (int c = blockColumn; c < blockColumn + 3; ++c) { // Visits its columns.
                if (cells[r][c] == value) // Checks for the same value in this box.
                    return false; // Rejects a repeated value in the box.
            }
        }
        return true; // The value passed all three Sudoku checks.
    }

    // look for the first empty cell from left to right, then top to bottom.
    // Then send its coordinates back to search.
    // This function only reads the grid, so it is const.
    bool findEmpty(int& row, int& column) const {
        for (int r = 0; r < 9; ++r) { // Visits rows from index 0 through 8.
            for (int c = 0; c < 9; ++c) { // Visits each column in the current row.
                if (cells[r][c] == 0) { // A zero identifies an empty cell.
                    row = r; // Sends the empty cell's row to the caller.
                    column = c; // Sends its column to the caller.
                    return true; // Stops as soon as the first blank is found.
                }
            }
        }
        return false; // Every cell is filled, so there is no blank to return.
    }

    // add one to the solution count by updating its decimal digits. Keeping
    // the count in a string avoids a fixed-width integer limit. A 9 becomes 0
    // and carries to the next digit on the left. If all digits are 9,
    // a leading 1 is added to the string.
    void countSolution() {
        for (std::size_t i = solutionCount.size(); i > 0; --i) { // Moves from right to left.
            if (solutionCount[i - 1] != '9') { // This digit can increase without a carry.
                ++solutionCount[i - 1]; // Increases the digit by one.
                return; // The addition is finished.
            }
            solutionCount[i - 1] = '0'; // Changes 9 to 0 and carries left.
        }
        solutionCount.insert(solutionCount.begin(), '1'); // Adds a leading 1 if all digits were 9.
    }

    // try digits 1 through 9 in the first empty cell and recursively fill
    // the remaining blanks. When the grid is full, count and print it right
    // away. Each guess is removed afterward so every possible solution can
    // be explored without storing completed grids in a list.
    void search() {
        int row = 0;  // Holds the empty cell's row for this recursive call.
        int column = 0;  // Holds the empty cell's column for this recursive call.
        if (!findEmpty(row, column)) { // No blank means a completed, valid grid.
            countSolution();          // Adds this completed grid to the total.
            std::cout << "Solution " << solutionCount << ":\n"; // Prints its number.
            printGrid(cells); // Prints the solution immediately.
            return; // Returns to the previous call to try other choices.
        }

        for (int value = 1; value <= 9; ++value) { // Try each possible Sudoku digit.
            if (allowed(row, column, value)) { // Continues only if the guess is legal.
                cells[row][column] = value; // Place the current guess in the grid.
                search(); // Explore all completions of this guess.
                cells[row][column] = 0; // Clears this guess before trying the next branch.
            }
        }
    }

    // read one line into a supplied row without changing the puzzle itself.
    // The line is passed by const reference to avoid copying it, and row is a
    // writable reference to one nine-cell array. It returns 0 for an empty line,
    // 9 for a full row, -1 for an invalid token or extra cell, and 1-8 for a
    // short row that readGrid will reject.
    static int readRow(const std::string& line, Grid::value_type& row) {
        std::istringstream input(line); // Makes the line readable as a stream.
        std::string token; // Holds one whitespace-separated cell token.
        int column = 0; // Tracks the next cell position to fill.
        while (input >> token) { // Reads tokens until this line has no more.
            if (column == 9 || token.size() != 1) // Rejects extra or multi-character cells.
                return -1; // Reports invalid input before indexing the row.
            char ch = token[0]; // Gets the single character in this cell's token.
            if (ch == '_') // An underscore represents an empty cell in the file.
                row[column] = 0; // Converts the blank to the internal zero value.
            else if (ch >= '1' && ch <= '9') // Accepts only clue digits 1 through 9.
                row[column] = ch - '0'; // Converts the digit character to an integer.
            else // Any other character, including a literal 0, is invalid.
                return -1; // Reports that the row contains an unsupported token.
            ++column; // Moves to the next column after accepting this cell.
        }
        return column; // Tells readGrid how many valid cells were read.
    }

    // check the given clues before searching. Each clue is temporarily
    // removed so allowed will compare it only with other cells. I restore it
    // before returning. If any clue conflicts with another,
    // the puzzle has no solution.
    bool checkClues() {
        for (int r = 0; r < 9; ++r) { // Visits every row in the loaded grid.
            for (int c = 0; c < 9; ++c) { // Visits each cell in this row.
                int value = cells[r][c]; // Saves the clue before temporarily clearing it.
                if (value == 0) // Empty cells have no clue to validate.
                    continue; // Moves to the next cell.
                cells[r][c] = 0; // Prevents the clue from matching itself.
                bool valid = allowed(r, c, value); // Checks the clue against its neighbors.
                cells[r][c] = value; // Restores the original clue immediately.
                if (!valid) // A repeated clue makes the puzzle unsolvable.
                    return false; // Stops once a conflict is found.
            }
        }
        return true; // All given clues satisfy the row, column, and box rules.
    }

    // read the whole file so extra rows or trailing junk are also checked.
    // Blank lines are skipped, but every nonblank line must contain nine cells.
    static bool readGrid(std::istream& input, Grid& loaded) {
        Grid::value_type nextRow{}; // Creates a zero-initialized row to hold the next nine cells.
        std::string line; // Holds each line read from the file.
        int row = 0; // Counts the puzzle rows stored so far.
        while (std::getline(input, line)) { // Reads until the file ends or a read fails.
            int found = readRow(line, nextRow); // Reads the line and gets its cell count.
            if (found == 0) // No tokens means the line is blank.
                continue; // Skips blank lines without counting them as puzzle rows.
            if (found != 9 || row == 9) // Checks for an invalid row or an extra row.
                return false; // Rejects the file if its format is wrong.
            loaded[row++] = nextRow; // Stores the row, then advances to the next row.
        }
        return !input.bad() && row == 9; // Requires nine rows and no badbit read error.
    }

public: // public functions used by SudokuProgram
    // load into a temporary grid so a failed read leaves the old puzzle unchanged.
    // After a successful read, replace the grid and check the new clues.
    // Conflicting clues are handled by solve as a puzzle with no solution.
    bool load(const std::string& filename) {
        std::ifstream input(filename); // Opens the named file for reading.
        if (!input) // Checks whether opening the file failed.
            return false; // Reports that the file could not be opened.
        Grid loaded{}; // Creates a temporary grid filled with zeros.
        if (!readGrid(input, loaded)) // Reads and checks the file's format.
            return false; // Leaves the old puzzle unchanged if the read fails.

        cells = loaded; // Replaces the stored grid after a successful read.
        cluesClash = !checkClues(); // Updates the conflict flag for the new puzzle.
        return true; // Reports successful loading, even if the clues conflict.
    }

    // reset the count and search only if the given clues do not conflict.
    // Search prints each solution, then this function prints the final result.
    void solve() {
        solutionCount = "0"; // Starts with a fresh solution count.
        if (!cluesClash) // Checks that the original clues do not conflict.
            search(); // Finds and prints every possible solution.
        if (solutionCount == "0") // No valid completion was found.
            std::cout << "No solution found\n"; // Prints the no-solution message.
        else // At least one solution was found.
            std::cout << "Solutions found: " << solutionCount << '\n'; // Prints the total.
    }

    // print the current puzzle without allowing changes to the grid.
    void printPuzzle() const {
        printGrid(cells); // Prints the stored puzzle using the shared grid printer.
    }

private: // private printing helper
    // print one space between cells and one line per row, using _ for blanks.
    // The const reference avoids copying the grid and prevents changes to it.
    static void printGrid(const Grid& grid) {
        for (int r = 0; r < 9; ++r) { // Prints rows from top to bottom.
            for (int c = 0; c < 9; ++c) { // Prints cells from left to right.
                if (c != 0) // Skips the space before the first cell in a row.
                    std::cout << ' '; // Adds a space between cells.
                std::cout << (grid[r][c] == 0 ? '_' : char('0' + grid[r][c])); // Prints the cell.
            }
            std::cout << '\n'; // Starts a new line after the completed row.
        }
    }
};

// keep filename selection and batch processing together here.
// Sudoku handles one puzzle at a time.
class SudokuProgram {
private: // private data
    std::vector<std::string> filenames; // Stores filenames in processing order.

public: // public functions used by main
    // use command-line filenames when supplied, otherwise use the five default files.
    // argc counts the arguments and argv stores them. Index 0 is the program name.
    SudokuProgram(int argc, char* argv[]) {
        if (argc > 1) { // Checks whether any filenames were supplied.
            for (int i = 1; i < argc; ++i) // Visits each filename argument in order.
                filenames.emplace_back(argv[i]); // Adds the filename to the list.
        } else { // Uses the default files when no filenames were supplied.
            for (int i = 1; i <= 5; ++i) // Generates file numbers 1 through 5.
                filenames.push_back("puzzle" + std::to_string(i) + ".txt"); // Builds each filename.
        }
    }

    // process each file with a new Sudoku object so puzzles stay separate.
    // A file error prints a message and moves on to the next file.
    // This function does not change the filename list, so it is const.
    int run() const {
        for (std::size_t i = 0; i < filenames.size(); ++i) { // Visits every filename.
            const std::string& filename = filenames[i]; // Uses the name without copying it.
            if (i != 0) // Checks whether this is after the first file.
                std::cout << '\n'; // Separates file results with a blank line.
            std::cout << filename << '\n'; // Prints the name of the current file.

            Sudoku puzzle; // Creates a new puzzle object for this file.
            if (!puzzle.load(filename)) { // Tries to read the puzzle file.
                std::cout << "Unable to read puzzle\n"; // Reports the failed load.
                continue; // Skips this file and moves to the next one.
            }
            puzzle.printPuzzle(); // Prints the input grid before solving it.
            puzzle.solve(); // Prints every solution and the final result.
        }
        return 0; // Reports normal completion after processing all files.
    }
};

// start the program and return its exit status to the operating system.
int main(int argc, char* argv[]) {
    SudokuProgram program(argc, argv); // Selects the files using the command-line arguments.
    return program.run(); // Runs the program and returns its exit status.
}
