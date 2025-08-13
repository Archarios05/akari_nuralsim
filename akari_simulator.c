#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> 

// --- データ構造の定義 ---
typedef enum {
    CELL_WHITE,   // 0: 白マス
    CELL_BLACK,   // 1: 黒マス (数字なし)
    CELL_BLACK_0, // 2: 黒マス (0)
    CELL_BLACK_1, // 3: 黒マス (1)
    CELL_BLACK_2, // 4: 黒マス (2)
    CELL_BLACK_3, // 5: 黒マス (3)
    CELL_BLACK_4  // 6: 黒マス (4)
} CellType;

typedef struct {
    CellType type;
    bool has_light; // 電球があるか (true: ある, false: ない)
    int is_lit;    // 照らされているか (true: 照らされている, false: いない)
} Cell;

// --- グローバル変数 ---
Cell** board = NULL;
int board_height = 0;
int board_width = 0;

// --- 関数のプロトタイプ宣言 ---
void initializeBoard(const char** puzzle_data, int h, int w);
void cleanupBoard();
void displayBoard();
bool placeLight(int row, int col);
bool removeLight(int row, int col);
void updateIllumination();
int checkPuzzleState(); // 0: 進行中, 1: クリア, <0: ルール違反

// --- メイン関数 ---
int main() {
    // 例として7x7の盤面データを定義
    board_height = 7;
    board_width = 7;
    const char* puzzle_sample[7] = {
        "_1_W_W_",
        "W_W_W_W",
        "__W0_W_",
        "WWWWWWW",
        "_W_2W__",
        "W_W_W_W",
        "_W_W_0_"
    }; // W: White, _: Black (no number), 0-4: Numbered Black

    // 1. 初期化
    initializeBoard(puzzle_sample, board_height, board_width);

    // 2. メインループ
    int state;
    char command;
    int r, c;

    while (true) {
        system("cls || clear"); // 画面をクリア (Windows/Linux)
        printf("--- Akari Puzzle Simulator ---\n");
        displayBoard();
        
        state = checkPuzzleState();
        printf("\nStatus: ");
        switch(state) {
            case 1:  printf("🎉 CONGRATULATIONS! Puzzle Solved! 🎉\n"); break;
            case 0:  printf("In Progress...\n"); break;
            case -1: printf("❌ ERROR: A light is being lit by another light.\n"); break;
            case -2: printf("❌ ERROR: A numbered block has the wrong number of adjacent lights.\n"); break;
        }

        if (state == 1) {
            break; // クリアしたらループを抜ける
        }

        printf("\nEnter command (p r c / r r c / q): ");
        printf("\n- p r c: Place light at (row, col)");
        printf("\n- r r c: Remove light from (row, col)");
        printf("\n- q: Quit\n> ");

        scanf(" %c", &command);

        if (command == 'q') {
            break;
        }

        if (command == 'p' || command == 'r') {
            scanf("%d %d", &r, &c);
            bool success = false;
            if (command == 'p') {
                success = placeLight(r, c);
            } else {
                success = removeLight(r, c);
            }
            if (success) {
                updateIllumination();
            } else {
                printf("Invalid move. Press Enter to continue.");
                while(getchar() != '\n'); // 入力バッファをクリア
                getchar();
            }
        }
    }

    // 3. メモリ解放
    cleanupBoard();
    printf("Program finished. Goodbye!\n");
    return 0;
}


/**
 * @brief 盤面データを読み込み、メモリを確保して初期化する
 */
void initializeBoard(const char** puzzle_data, int h, int w) {
    board_height = h;
    board_width = w;

    // 盤面のメモリ確保 (行のポインタ配列)
    board = (Cell**)malloc(h * sizeof(Cell*));
    for (int i = 0; i < h; i++) {
        // 各行のメモリ確保 (セルの配列)
        board[i] = (Cell*)malloc(w * sizeof(Cell));
    }

    // 各セルを初期化
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            char cell_char = puzzle_data[i][j];
            board[i][j].has_light = false;
            board[i][j].is_lit = false;

            switch (cell_char) {
                case 'W': board[i][j].type = CELL_WHITE; break;
                case '_': board[i][j].type = CELL_BLACK; break;
                case '0': board[i][j].type = CELL_BLACK_0; break;
                case '1': board[i][j].type = CELL_BLACK_1; break;
                case '2': board[i][j].type = CELL_BLACK_2; break;
                case '3': board[i][j].type = CELL_BLACK_3; break;
                case '4': board[i][j].type = CELL_BLACK_4; break;
                default:  board[i][j].type = CELL_WHITE; break; // 不明な文字は白マス扱い
            }
        }
    }
}

/**
 * @brief 盤面表示用の文字を取得するヘルパー関数
 */
char getCellChar(Cell cell) {
    if (cell.has_light) return '@'; // 電球
    
    switch (cell.type) {
        case CELL_WHITE:   return cell.is_lit ? '*' : '.'; // 照らされている白マス / 白マス
        case CELL_BLACK:   return ' '; // 黒マス
        case CELL_BLACK_0: return '0';
        case CELL_BLACK_1: return '1';
        case CELL_BLACK_2: return '2';
        case CELL_BLACK_3: return '3';
        case CELL_BLACK_4: return '4';
    }
    return '?';
}

/**
 * @brief 現在の盤面をコンソールに表示する
 */
void displayBoard() {
    printf("  "); // 列番号のためのオフセット
    for (int j = 0; j < board_width; j++) {
        printf("%d ", j);
    }
    printf("\n");

    for (int i = 0; i < board_height; i++) {
        printf("%d ", i); // 行番号
        for (int j = 0; j < board_width; j++) {
            printf("%c ", getCellChar(board[i][j]));
        }
        printf("\n");
    }
}

/**
 * @brief 指定した座標に電球を置く
 * @return 成功した場合はtrue, 失敗した場合はfalse
 */
bool placeLight(int row, int col) {
    if (row < 0 || row >= board_height || col < 0 || col >= board_width) {
        return false; // 盤面外
    }
    if (board[row][col].type != CELL_WHITE) {
        return false; // 白マス以外には置けない
    }
    if (board[row][col].has_light) {
        return false; // 既に電球がある
    }
    
    board[row][col].has_light = true;
    return true;
}

/**
 * @brief 指定した座標の電球を削除する
 * @return 成功した場合はtrue, 失敗した場合はfalse
 */
bool removeLight(int row, int col) {
    if (row < 0 || row >= board_height || col < 0 || col >= board_width) {
        return false; // 盤面外
    }
    if (!board[row][col].has_light) {
        return false; // 電球がない
    }
    
    board[row][col].has_light = false;
    return true;
}

/**
 * @brief 照明の状態を更新する
 */
void updateIllumination() {
    // 1. 全ての照明をリセット
    for (int i = 0; i < board_height; i++) {
        for (int j = 0; j < board_width; j++) {
            board[i][j].is_lit = false;
        }
    }

    // 2. 各電球から光を伸ばす
    for (int i = 0; i < board_height; i++) {
        for (int j = 0; j < board_width; j++) {
            if (board[i][j].has_light) {
                board[i][j].is_lit = true; // 電球自身も照らされる
                
                // 上へ
                for (int k = i - 1; k >= 0 && board[k][j].type == CELL_WHITE; k--) board[k][j].is_lit = true;
                // 下へ
                for (int k = i + 1; k < board_height && board[k][j].type == CELL_WHITE; k++) board[k][j].is_lit = true;
                // 左へ
                for (int k = j - 1; k >= 0 && board[i][k].type == CELL_WHITE; k--) board[i][k].is_lit = true;
                // 右へ
                for (int k = j + 1; k < board_width && board[i][k].type == CELL_WHITE; k++) board[i][k].is_lit = true;
            }
        }
    }
}

/**
 * @brief パズルの状態をチェックする
 * @return 1:クリア, 0:進行中, -1:電球違反, -2:数字違反
 */
int checkPuzzleState() {
    bool all_white_lit = true;

    for (int i = 0; i < board_height; i++) {
        for (int j = 0; j < board_width; j++) {
            // ルール違反1: 電球が他の電球に照らされているか
            if (board[i][j].has_light && board[i][j].is_lit) {
                // ただし、自分自身の光は許容する。他の電球からの光か判定する。
                // (この実装ではupdateIlluminationで全電球をis_lit=trueにしているので、より厳密な判定が必要)
                // 簡単な判定: 自分以外の電球が同じ行/列にあるかチェック
                // 上
                for(int k=i-1; k>=0; k--) {
                    if(board[k][j].has_light) return -1;
                    if(board[k][j].type != CELL_WHITE) break;
                }
                // 下
                for(int k=i+1; k<board_height; k++) {
                    if(board[k][j].has_light) return -1;
                    if(board[k][j].type != CELL_WHITE) break;
                }
                // 左
                for(int k=j-1; k>=0; k--) {
                    if(board[i][k].has_light) return -1;
                    if(board[i][k].type != CELL_WHITE) break;
                }
                // 右
                for(int k=j+1; k<board_width; k++) {
                    if(board[i][k].has_light) return -1;
                    if(board[i][k].type != CELL_WHITE) break;
                }
            }

            // ルール違反2: 数字付き黒マスの周りの電球の数
            if (board[i][j].type >= CELL_BLACK_0 && board[i][j].type <= CELL_BLACK_4) {
                int required_lights = board[i][j].type - CELL_BLACK_0;
                int adjacent_lights = 0;
                // 上
                if (i > 0 && board[i-1][j].has_light) adjacent_lights++;
                // 下
                if (i < board_height - 1 && board[i+1][j].has_light) adjacent_lights++;
                // 左
                if (j > 0 && board[i][j-1].has_light) adjacent_lights++;
                // 右
                if (j < board_width - 1 && board[i][j+1].has_light) adjacent_lights++;
                
                if (adjacent_lights != required_lights) return -2;
            }
            
            // クリア条件: 全ての白マスが照らされているか
            if (board[i][j].type == CELL_WHITE && !board[i][j].is_lit) {
                all_white_lit = false;
            }
        }
    }

    if (all_white_lit) {
        return 1; // クリア！
    }

    return 0; // まだ進行中
}


/**
 * @brief 確保した盤面のメモリを解放する
 */
void cleanupBoard() {
    if (board != NULL) {
        for (int i = 0; i < board_height; i++) {
            free(board[i]); // 各行を解放
        }
        free(board); // ポインタ配列を解放
        board = NULL;
    }
}