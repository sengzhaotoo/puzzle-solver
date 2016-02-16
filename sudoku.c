#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sudoku.h"

#define NUM_DIGITS 9
#define NUM_ROWS   NUM_DIGITS
#define NUM_COLS   NUM_DIGITS
#define NUM_PEERS  20
#define NUM_UNITS  3
#define DIGITS     "123456789"
#define ROW_NAMES  "ABCDEFGHI"
#define COL_NAMES  DIGITS

typedef struct square {
    char vals[NUM_DIGITS+1]; // string of possible values
    unsigned char row;
    unsigned char col;
    struct square *peers[NUM_PEERS];
    struct square *units[NUM_UNITS][NUM_DIGITS];
} square_t;

typedef struct puzzle {
    square_t squares[NUM_ROWS][NUM_COLS];
} puzzle_t;

void solve(unsigned char grid[9][9]);

// following are static ("private") function declarations --- add as needed

static puzzle_t *create_puzzle(unsigned char grid[9][9]);
static void init_peers(puzzle_t *puz, int row, int col);
static puzzle_t *copy_puzzle(puzzle_t *puz);
static void free_puzzle(puzzle_t *puz);
static void print_puzzle(puzzle_t *);

static puzzle_t *search(puzzle_t *puz);
static puzzle_t *assign(puzzle_t *puz, int row, int col, char val);
static puzzle_t *eliminate(puzzle_t *puz, int row, int col, char val);
static void delete(char *string, char c);
/*************************/
/* Public solve function */
/*************************/

void solve(unsigned char grid[9][9]) {
    puzzle_t *puz = create_puzzle(grid);
    puzzle_t *solved;
    if ((solved = search(puz)) != NULL) {
        print_puzzle(solved);
    }
    free_puzzle(solved);
}

/*******************************************/
/* Puzzle data structure related functions */
/*******************************************/

static puzzle_t *create_puzzle(unsigned char vals[9][9]) {
    puzzle_t *newpuz = malloc(sizeof(puzzle_t)); //malloc the size of the type
    square_t *square;
    int i,j;
    for(i = 0; i < NUM_ROWS; i++){
        for(j = 0; j < NUM_COLS; j++){
            square = &(newpuz->squares[i][j]);
            strcpy(square->vals, DIGITS);
            square->row = i;
            square->col = j;
            init_peers(newpuz, square->row, square->col);
        }
    }
    if(vals != NULL){
        for(i = 0; i < NUM_ROWS; i++){
            for(j = 0; j < NUM_COLS; j++){
                assign(newpuz, i, j, vals[i][j]);
            }
        }
    }
    return newpuz; // supposedly free in solve()
}

static void init_peers(puzzle_t *puz, int row, int col) {
    int i, j, r, c, unitNum = 0, peerNum = 0;
    square_t *square = &(puz->squares[row][col]);
    
    // Columns
    for(i = 0; i < NUM_COLS; i++){
        square->units[0][i] = &(puz->squares[row][i]);
        if(i != col){
            square->peers[peerNum++] = &(puz->squares[row][i]);
        }
    }
    
    // Rows
    for(i = 0; i < NUM_ROWS; i++){
        square->units[1][i] = &(puz->squares[i][col]);
        if(i != row){
            square->peers[peerNum++] = &(puz->squares[i][col]);
        }
    }
    
    r = (row/3)*3;
    c = (col/3)*3;
    
    // Box Peers
    for(i = 0; i < 3; i++){
        for(j = 0; j < 3; j++){
            square->units[2][unitNum++] = &(puz->squares[i+r][j+c]);
            if((r + i != row) && (c + j != col)){
                square->peers[peerNum++] = &(puz->squares[r+i][c+j]);
            }
        }
    }
}

static void free_puzzle(puzzle_t *puz) {
    free(puz);
}

static puzzle_t *copy_puzzle(puzzle_t *puz) {
    puzzle_t *copy = create_puzzle(NULL);
    int i, j;
    for(i = 0; i < NUM_ROWS; i++){
        for(j = 0; j < NUM_COLS; j++){
            strcpy(copy->squares[i][j].vals, puz->squares[i][j].vals);
        }
    }
    return copy;
}

void print_puzzle(puzzle_t *p) {
    int i, j;
    for (i=0; i<NUM_ROWS; i++) {
        for (j=0; j<NUM_COLS; j++) {
            //printf(" %9s", p->squares[i][j].vals); // may be useful while debugging
            printf(" %2s", p->squares[i][j].vals);
        }
        printf("\n");
    }
}

/**********/
/* Search */
/**********/

static puzzle_t *search(puzzle_t *puz) {
    puzzle_t *puzzleCopy = NULL, *value = NULL;
    square_t *square = NULL, *found = NULL;
    int i, j, k, min = 9;
    char val;
    for(i = 0; i < NUM_ROWS; i++){
        for(j = 0; j < NUM_COLS; j++){
            square = &(puz->squares[i][j]);
            if(strlen(square->vals) == 0){
                return NULL;
            } else if(strlen(square->vals) > 1){
                if(min > strlen(square->vals)){
                    min = (int) strlen(square->vals);
                    found = square;
                }
            }
        }
    }
    if(found == NULL){
        return puz;
    } else {
        for(k = 0; k < strlen(square->vals); k++){
            val = found->vals[k];
            puzzleCopy = copy_puzzle(puz);
            
            // recursive search for solutions
            value = search(assign(puzzleCopy, found->row, found->col, val));
            if(value != NULL){
                free_puzzle(puz);
                return value;
            } else {
                free_puzzle(puzzleCopy);
            }
        }
    }
    free_puzzle(puz);
    free_puzzle(value);
    return puzzleCopy; // supposedly free in solve()
}

/**************************/
/* Constraint propagation */
/**************************/

static puzzle_t *assign(puzzle_t *puz, int row, int col, char val) {
    square_t *square;
    square = &(puz->squares[row][col]);
    int i;
    char left[NUM_DIGITS+1];
    if(strlen(square->vals) == 1){
        for(i = 0; i < NUM_PEERS; i++){
            if(strchr(square->peers[i]->vals, val)){
                if(strlen(square->peers[i]->vals) == 1){
                    return NULL;
                } else {
                    eliminate(puz, square->peers[i]->row, square->peers[i]->col, val);
                }
            }
        }
    }
    if(strchr(square->vals, val)){
        strcpy(left, square->vals);
        delete(left, val);
        for(i = 0; i < (strlen(left)); i++) {
            eliminate(puz, row, col, left[i]);
        }
    }
    return puz;
}

static puzzle_t *eliminate(puzzle_t *puz, int row, int col, char val) {
    square_t *square, *boxes;
    square = &(puz->squares[row][col]);
    int i, j, boxRow = 0, boxCol = 0, count = 0;
    if(!strchr(square->vals, val)){
        return puz;
    } else {
        delete(square->vals, val);
    }
    
    if(strlen(square->vals) == 0){
        return NULL;
    } else if(strlen(square->vals) == 1){
        assign(puz, row, col, square->vals[0]);
    }
    
    for(i = 0; i < NUM_UNITS; i++){
        for(j = 0; j < NUM_DIGITS; j++){
            boxes = square->units[i][j];
            if(strchr(boxes->vals, val)){
                count++;
                boxRow = boxes->row;
                boxCol = boxes->col;
            }
        }
    }
    if(count == 1){
        assign(puz, boxRow, boxCol, val);
    }
    return puz;
}

/*****************************************/
/* Misc (e.g., utility) functions follow */
/*****************************************/
//Type: "head -10 p096_sudoku.txt | ./sudoku" to run
//Function to delete a character from a string and shift characters

static void delete(char *string, char c){
    char *src, *dst;
    for(src = dst = string; *src != '\0'; src++){
        *dst = *src;
        if(*dst != c){
            dst++;
        }
    }
    *dst = '\0';
}
