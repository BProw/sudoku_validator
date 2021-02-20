/*  
    Sudoku Solution Validator
    HW03, CS 3600, Operating Systems, Summer 2020
    @author Brian LeProwse 
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void valid(int x[], int y[], int z[]);
void readFile();                   // Func reads data from input file.
void *columnThreads(void *param);  // Worker threads for 9 columns.
void *rowThreads(void *param);     // Worker threads for 9 rows.
void *subgridThreads(void *param); // Worker threads for 9 subgrids.

/* col, row, subgrid displayMsg funcs output details of each assigned thread */
void colDisplayMsg(pthread_t tid, int iVal, int startRow,
                   int endCol, int left, int right);

void rowDisplayMsg(pthread_t tid, int iVal, int startRow,
                   int endCol, int left, int right);

void subgridDisplayMsg(pthread_t tid, int iVal, int startRow,
                       int endCol, int left, int right);

// Parent thread output TID vals & validity of each.
void parentDisplay(pthread_t tid[]);

int matchNums(int a[]); // Func finds any duplicate vals.

#define CHILD_THREADS 27

typedef int bool; // Constants for boolean vals.
#define TRUE 1
#define FALSE 0

FILE *file; // Sudoku puzzle input file.

int sudokuPuzzle[9][9]; // 2D array containing all elements in puzzle.

// 9-element bool arrs for cols, rows, subgrids (all init to 0).
int colBoolArr[9] = {0};
int rowBoolArr[9] = {0};
int subgridBoolArr[9] = {0};

long tid_col[9];     // Thread ID of each column.
long tid_row[9];     // Thread ID of each row.
long tid_subgrid[9]; // Thread ID of each subgrid.

typedef struct // 5 fields for the 27 created threads.
{
    int topRow;
    int bottomRow;
    int leftColumn;
    int rightColumn;
    int iValue;
} parameters; // Structure reference.

int main(int argc, char *argv[])
{
    int i, k, j;
    bool x = TRUE,
         y = FALSE;

    readFile(); // Read input file.

    pthread_t tid[CHILD_THREADS];           // Identifiers for each thread.
    pthread_attr_t attr[CHILD_THREADS];     // Thread attributes (default).
    parameters threadParams[CHILD_THREADS]; // Structure variable.

    if (argc != 2)
    {
        fprintf(stderr, "usage: < 0 or positie integer value\n");
          
            /*exit(1);*/
            return -1;
    }
    if (atoi(argv[1]) < 0)
    {
        fprintf(stderr, "Argument %d must be nonnegative\n", atoi(argv[1]));
        /*exit(1);*/
        return -1;
    }

    for (i = 0; i < CHILD_THREADS; i++) // Provide thread attributes.
    {
        pthread_attr_init(&(attr[i]));
    }

    for (i = 0; i < CHILD_THREADS; i++)
    {
        pthread_join(tid[i], NULL);
    }
    // Init the 9 structures for the 9 columns of puzzle.
    for (i = 0, k = 1; i < 9; i++, k++)
    {
        threadParams[i].topRow = 0;
        threadParams[i].bottomRow = 8;
        threadParams[i].leftColumn = k - 1;
        threadParams[i].rightColumn = k - 1;
        threadParams[i].iValue = i;

        pthread_create(&(tid[i]), // Create column worker threads.
                       &(attr[i]),
                       columnThreads,
                       &(threadParams[i]));

        tid_col[i] = (unsigned long)(tid[i]);
    }
    for (i = 0; i < 9; i++)
    {
        pthread_join(tid[i], NULL);
    }
    // Init the 9 structures for the 9 rows of puzzle.
    for (i = 0, k = 1; i < 9; i++, k++)
    {
        threadParams[i].topRow = k - 1;
        threadParams[i].bottomRow = k - 1;
        threadParams[i].leftColumn = 0;
        threadParams[i].rightColumn = 8;
        threadParams[i].iValue = i;

        pthread_create(&(tid[i]), // Create row worker threads.
                       &(attr[i]),
                       rowThreads,
                       &(threadParams[i]));

        tid_row[i] = (unsigned long)(tid[i]);
    }
    for (i = 0; i < 9; i++)
    {
        
        pthread_join(tid[i], NULL);
    }
    // Init the 9 structures for the 9 subgrids of puzzle.
    int b;

    for (b = 0, i = 0, k = 0; i < 9; i += 3, k += 3, b++)
    {

        threadParams[i].topRow = i;
        threadParams[i].bottomRow = k;
        threadParams[i].leftColumn = i;
        threadParams[i].rightColumn = k;
        threadParams[i].iValue = i;

        pthread_create(&(tid[i]), // Create subgrid worker threads.
                       &(attr[i]),
                       subgridThreads,
                       &(threadParams[i]));

        tid_subgrid[i] = (unsigned long)(tid[i]);
    }
    for (i = 0; i < 9; i++)
    {
        pthread_join(tid[i], NULL);
    }
  
    parentDisplay(tid);
    valid(colBoolArr, rowBoolArr, subgridBoolArr);
    return 0;
}

/* Read Sudoku .txt file data into 2D array then display contents.*/
void readFile()
{
    int i, j;
    file = fopen("SudokuPuzzle.txt", "r"); //"r" = read mode. "w" = write mode...
    char digit[] = " ";
    for (i = 0; i < 9; i++)
    {
        for (j = 0; j < 9; j++)
        {
            fscanf(file, " %s", &digit[0]);
            // Convert read char to int (atoi(digit)).
            sudokuPuzzle[i][j] = atoi(digit);
        }
    }
    for (i = 0; i < 9; i++)
    {
        for (j = 0; j < 9; j++)
        {

            printf("%d ", sudokuPuzzle[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}
void *columnThreads(void *param)
{

    int iVal, startRow, endCol, left, right;
    int colArr[9] = {0};
    int i, j, colValue;
    parameters *data = (parameters *)malloc(sizeof(parameters));
    data = (parameters *)param;
    pthread_t self;
    self = pthread_self();
    startRow = data->topRow;
    endCol = data->bottomRow;
    left = data->leftColumn;
    right = data->rightColumn;
    iVal = data->iValue;

    for (i = startRow; i <= endCol; i++)
    {
        for (j = left; j <= right; j++)
        {
            colValue = sudokuPuzzle[i][j];
            // printf("%d ",colValue);
            colArr[i] = colValue;
        }
    }
    colBoolArr[iVal] = matchNums(colArr);
    colDisplayMsg(self, iVal, startRow, endCol, left, right);
    pthread_exit(0);
}

void *rowThreads(void *param)
{
    int iVal, startRow, endCol, left, right;
    int rowArr[9] = {0};
    int i, j, rowValue;
    parameters *data = (parameters *)malloc(sizeof(parameters));
    data = (parameters *)param;
    pthread_t self;
    self = pthread_self();
    startRow = data->topRow;
    endCol = data->bottomRow;
    left = data->leftColumn;
    right = data->rightColumn;
    iVal = data->iValue;

    for (i = left; i <= right; i++)
    {
        for (j = endCol; j <= startRow; j++)
        {
            rowValue = sudokuPuzzle[j][i];
            // printf("%d ",rowValue);
            rowArr[i] = rowValue;
        }
    }
    rowBoolArr[iVal] = matchNums(rowArr);
    rowDisplayMsg(self, iVal, startRow, endCol, left, right);
    pthread_exit(0);
}

void *subgridThreads(void *param)
{
    int iVal, startRow, endCol, left, right;
    int subgridArr[9] = {0};
    int i, j, subgridValue;
    parameters *data = (parameters *)malloc(sizeof(parameters));
    data = (parameters *)param;
    pthread_t self;
    self = pthread_self();
    startRow = data->topRow;
    endCol = data->bottomRow;
    left = data->leftColumn;
    right = data->rightColumn;
    iVal = data->iValue;

    // printf("StartROW: %d  endCol: %d   left: %d  right %d ", startRow, endCol, left, right);
    for (i = left; i < left + 3; i++)
    {
        for (j = endCol; j < endCol + 3; j++)
        {
            subgridValue = sudokuPuzzle[i][j];
            // printf("%d ",subgridValue);
            subgridArr[i] = subgridValue;
        }
    }

    subgridBoolArr[iVal] = matchNums(subgridArr);

    subgridDisplayMsg(self, iVal, startRow, endCol, left, right);
    pthread_exit(0);
}
/*  Check for duplicate #s in col, row, or subgrid.
*   @return FALSE duplicates found.
*   @return TRUE no duplicates. 
*/
int matchNums(int a[])
{

    int i, j;
    for (i = 0; i < 9; i++)
    {
        // printf("%d ", a[i]);
    }
    //printf("\n");
    for (i = 0; i < 9; i++)
    {
        for (j = i + 1; j < 9; j++)
        {
            if (a[i] == a[j])
            {
                return FALSE;
            }
        }
    }
    return TRUE;
}
/*   Output col TID, top & bottom row vals, and left & right col vals. */
void colDisplayMsg(pthread_t self, int iVal,
                   int startRow, int endCol,
                   int left, int right)
{
    if (colBoolArr[iVal] == 1)
    {
        printf("TID: %lX TRow: %d, BRow: %d, LCol: %d, RCol: %d %s\n",
               (unsigned long)self,
               startRow,
               endCol,
               left,
               right,
               "Valid!");
    }
    else
    {
        printf("TID: %lX TRow: %d, BRow: %d, LCol: %d, RCol: %d %s\n",
               (unsigned long)self,
               startRow,
               endCol,
               left,
               right,
               "Invalid!");
    }
}
/*  Output row TID, top & bottom row vals, and left & right col vals. */
void rowDisplayMsg(pthread_t self, int iVal,
                   int startRow, int endCol,
                   int left, int right)
{
    if (rowBoolArr[iVal] == 1)
    {
        printf("TID: %lX TRow: %d, BRow: %d, LCol: %d, RCol: %d %s\n",
               (unsigned long)self,
               startRow,
               endCol,
               left,
               right,
               "Valid!");
    }
    else
    {
        printf("TID: %lX TRow: %d, BRow: %d, LCol: %d, RCol: %d %s\n",
               (unsigned long)self,
               startRow,
               endCol,
               left,
               right,
               "Invalid!");
    }
}
/*  Output subgrid TID, top & bottom row vals, and left & right col vals. */
void subgridDisplayMsg(pthread_t self, int iVal,
                       int startRow, int endCol,
                       int left, int right)
{
    if (subgridBoolArr[iVal] == 1)
    {
        printf("TID: %lX TRow: %d, BRow: %d, LCol: %d, RCol: %d %s\n",
               (unsigned long)self,
               startRow,
               endCol,
               left,
               right,
               "Valid!");
    }
    else
    {
        printf("TID: %lX TRow: %d, BRow: %d, LCol: %d, RCol: %d %s\n",
               (unsigned long)self,
               startRow,
               endCol,
               left,
               right,
               "Invalid!");
    }
}
/* Output tid values for col, row, and subgrid and validity. */
void parentDisplay(pthread_t tid[])
{
    int i;

    for (i = 0; i < 9; i++)
    {
        if (colBoolArr[i] == 1)
        {
            printf("COLUMN: %lX %s", (unsigned long)tid_col[i], " Valid\n");
        }
        else
        {
            printf("COLUMN: %lX %s", (unsigned long)tid_col[i], " Invalid\n");
        }
    }
    for (i = 0; i < 9; i++)
    {
        if (rowBoolArr[i] == 1)
        {
            printf("ROW: %lX %s", (unsigned long)tid_row[i], " Valid\n");
        }
        else
        {
            printf("ROW: %lX %s", (unsigned long)tid_row[i], " Invalid\n");
        }
    }
    for (i = 0; i < 9; i++)
    {
        if (subgridBoolArr[i] == 1)
        {
            printf("SUBGRID: %lX %s",
                   (unsigned long)tid_subgrid[i], " Valid\n");
        }
        else
        {
            printf("SUBGRID: %lX %s",
                   (unsigned long)tid_subgrid[i], " Invalid\n");
        }
    }
}
void valid(int x[], int y[], int z[]) {
    int m, flag;
    for(m = 0; m < 9; m++) {
        if(x[m] == 1 && y[m] == 1 && z[m] == 1) {
            flag++;
        }
    }
  
    if(flag ==  1) {
        printf("\nSudoku Puzzle:  valid");
    } else {
        printf("\nSudoku Puzzle:  invalid");
    }
}
