# include <stdio.h>
# include <stdlib.h>
# include <stdbool.h>
# define SPACE 32
# define TAB 9
# define CR 13
# define LF 10
# define MMAXVAL 65536

struct per {int x, y;};

struct Queue { // FIRST IN FIRST OUT 
    int front, back, size, capacity; //position for the first and last element in the array, size and allocated capacity
    struct per *v; //array of struct per
};
typedef struct Queue Queue;
typedef struct per per;


//==========QUEUE FUNCTIONS==========

//allocates memory to queue structure and array
Queue *createQueue(int capacity) {
    Queue *q = (Queue *) malloc(sizeof(Queue));
    q -> capacity = capacity; 
    q -> front = 0, q -> size = 0, q -> back = capacity - 1;
    q -> v = (per *) malloc(sizeof(per) * (q -> capacity));
    return q;
}

// checks if queue size > capacity
bool fullQueue(Queue *q) { 
    if (q -> capacity == q -> size) return true;
    return false;
}

// check if queue size == 0
bool emptyQueue(Queue *q) { 
    if (!(q -> size)) return true;
    return false;
}

//push element in queue
Queue *pushBack(Queue *q, per val) { 
    if (fullQueue(q)) {
        printf("ERROR queue full\n");
        exit(1);
    }

    int mod = q -> capacity;
    q -> back = (q -> back + 1) % mod;
    q -> v[q -> back] = val;
    q -> size++;

    return q;
}

//get the first element waiting in the queue
per getFront(Queue *q) {
    return q -> v[q -> front];
}

//removes the first element waiting in the queue
Queue *popFront(Queue *q) {
    if (emptyQueue(q)) {
        printf("ERROR queue empty\n");
        exit(1);
    }
    int mod = q -> capacity;
    q -> front = (q -> front + 1) %  mod;
    q -> size--;

    return q;
}

void freeQueue(Queue *q) {
    free (q -> v);
    free (q);
}

int max(int x, int y) {
    if (x > y) return x;
    return y; 
}

void invalidFile() {
    fprintf(stderr, "Invalid .PGM file !\n");
    exit(1);
}

void Fill(int m[205][205], int fill[205][205], int x, int y, int nr) {
    int dx[4] = {0, 0, 1, -1};
    int dy[4] = {-1, 1, 0, 0};
    int cx, cy;
    per pos;
    pos.x = x, pos.y = y;
    fill[x][y] = nr;

    Queue *q = createQueue(200 * 200 + 10);
    q = pushBack(q, pos); 
    
    while (!emptyQueue(q)) {
        per current = getFront(q);
        int x = current.x, y = current.y;
        for (int i = 0; i < 4; ++i) {
            cx = x + dx[i];
            cy = y + dy[i];

            if (cx >= 0 && cx < 200 && cy >= 0 && cy < 200)
                if (m[x][y] == m[cx][cy] && !fill[cx][cy]) current.x = cx, current.y = cy, q = pushBack(q, current), fill[cx][cy] = nr;
        }

        q = popFront(q);
    }

    freeQueue(q);
}

void memsetFill(int fill[205][205]) {
    for (int i = 0; i < 200; ++i)
         for (int j = 0; j < 200; ++j)
            fill[i][j] = 0;
}

void checkFile(FILE *in) {
    if (in == NULL) {
        fprintf(stderr, "Cannot read pgm file\n");
		exit(1);
    }
}

void maxValCase1(FILE *in, int m[205][205]) {
    
    unsigned char b;

    for (int i = 0; i < 200; ++i) {
        for (int j = 0; j < 200; ++j) {
            b = fgetc(in);
            m[i][j] = b;
        }
    }

}

void countBlocks(int m[205][205], int fill[205][205]) {
    int nr = 0;

    for (int i = 0; i < 200; ++i)
        for (int j = 0; j < 200; ++j)
            if (!fill[i][j]) Fill(m, fill, i, j, ++nr);
   
}

unsigned char data1 (int x) {
    unsigned char c = 0;
    c |= (1 << 7), c |= (1 << 6);
    for (int j = 6; j < 12; ++j)
        if ( (x >> j) % 2 == 1 ) c |= (1 << (j - 6));

    return c;
}

unsigned char data2 (int x) {
    unsigned char c = 0;
    c |= (1 << 7), c |= (1 << 6); 
    for (int j = 0; j < 6; ++j)
        if ((x >> j) % 2 == 1) c |= (1 << j);
    
    return c;
}

void buildCommands(int fill[205][205], int noBlocks) {
    FILE *out = fopen("converted.sk", "wb");

    bool up, down, BREAK;
    int dx, dy, x, y;
    unsigned char v[1000];
    int a, b, c, d;
    a = 0, b = 0, c = 0, d = 0; // x, y, tx, ty
    
    v[0] = 0; // the array with the commands
    const unsigned char toolNone = 0x80; // 10000000
    const unsigned char toolBlock = 0x82; // 10000010
  //  const unsigned char toolColour = 0x83; // 10000011
    const unsigned char toolTargetX = 0x84; // 10000100
    const unsigned char toolTargetY = 0x85; // 10000101
    const unsigned char toolNFrame = 0x88;
    
    for (int color = 1; color <= noBlocks; ++color) {
        up = true, down = true; BREAK = true;
        for (int i = 0; i < 200 && BREAK; ++i)
            for (int j = 0; j < 200 && BREAK; ++j) {
                if (fill[i][j] == color && up == true && ((j + 1 == 200) || fill[i][j + 1] != color)) up = false, dx = i, dy = j;
                if (fill[i][j] == color && down == true && ( (i + 1 == 200) || fill[i + 1][j] != color) ) down = false, x = i, y = j;
                if (!up && !down) BREAK = true;
            }
    
        v[++v[0]] = toolNone;
        unsigned char c = 0;

        //update DATA for X
        v[++v[0]] = data1(x), v[++v[0]] = data2(x), v[++v[0]] = toolTargetX;

        //update DATA for Y
        v[++v[0]] = data1(y), v[++v[0]] = data2(y), v[++v[0]] = toolTargetY;

        //DY = 0 => x = TARGETX, y = TARGETY
        v[++v[0]] = toolNone;
        c = 0, c |= (1 << 6), v[++v[0]] = c;

         /*   //TX = 0, TY = 0
            int ans = data1(x);
            v[++v[0]] = data1(x);
            v[++v[0]] = data2(x);
            
            v[++v[0]] = toolTargetX;*/

        //update DATA for DX
        v[++v[0]] = data1(dx), v[++v[0]] = data2(dx), v[++v[0]] = toolTargetX;
            
        //update DATA for DY
        v[++v[0]] = data1(dy), v[++v[0]] = data2(dy), v[++v[0]] = toolTargetY;
        
        //drawBlock
        v[++v[0]] = toolBlock;
        c = 0, c |= (1 << 6), v[++v[0]] = c;

        //nextframe
        v[++v[0]]= toolNFrame;

       // printf("%d %d %d %d\n", x, y, dx, dy);
    }

    for (int i = 1; i <= v[0]; ++i) v[i - 1] = v[i];
    fwrite(v, 1, v[0], out); //write file
    printf("You successfully converted your file.\nThe new converted filename is 'converted.sk'\n");
}

void readFile(const char fileName[]) {
    FILE *in = fopen(fileName, "rb");
    checkFile(in);

    unsigned char b = fgetc(in);
    int i = 0, maxval, noBlocks = 0;
    int m[205][205], fill[205][205];
    bool ok = false, Continue = true;

    while (! feof(in) && Continue) {
        ++i;
        if (i == 1 && b != 'P') invalidFile(); //FORMAT
        if (i == 2 && b != '5') invalidFile(); //FORMAT
        if ((i == 3 || i == 7 || i == 11) && (b != SPACE && b != TAB && b != CR && b != LF)) invalidFile(); //WHITESPACE
        if ((i == 4 || i == 8) && b != '2') invalidFile(); //PIXELS 200x200
        if ((i == 5 || i == 6 || i == 9 || i == 10) && b != '0') invalidFile(); //PIXELS 200x200
        if (i == 11) maxval = 0; //initialise maxval
        b = fgetc(in);

        while (! feof(in) && !(b == SPACE || b == TAB || b == CR || b == LF) && i == 11) {
            if ('0' <= b && b <= '9') maxval = maxval * 10 + (b - '0');
                else invalidFile();
            ok = true; 
            b = fgetc(in);
        }

        if (!(0 <= maxval && maxval <= MMAXVAL) && ok) invalidFile();
        if (i == 11 && (b != SPACE && b != TAB && b != CR && b != LF) ) invalidFile();
        if (ok) Continue = false;
    }

    if (maxval < 256) maxValCase1(in, m);
        //else maxValCase2(in, m);
    
    memsetFill(fill); //set all values of matrix "fill" to 0 
    
    countBlocks(m, fill);
   
    for (int i = 0; i < 200; ++i)
        for (int j = 0; j < 200; ++j)
            noBlocks = max(noBlocks, fill[i][j]);
    
    buildCommands(fill, noBlocks);

}

int main (int n, char *args[n])
{   
    if (n != 2) printf("Input should be ./convert filename.pgm\n");
    if (n == 2) readFile(args[1]);
   
    return 0;
}
