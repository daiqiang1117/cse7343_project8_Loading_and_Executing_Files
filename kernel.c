/*        kernel.c
        Implement printString, readString, readSector and a simple own interrupt
        
        Qiang Dai
*/

/* macro */
#define LENGTH_LINE 80
#define LENGTH_SECTOR 512
#define NUM_SECTORS 26
#define LENGTH_FILE_NAME 6
#define SIZE_BUFFER 13312

/* Function Prototypes*/
void printString (char* word);
void readString (char* line);
void readSector (char* buffer, int sector);
void readFile(char* buffer, char* fileName);
int mod (int dividend, int divisor);
int div (int dividend, int divisor);
int matchFileName(char* buf, char* filename, int bufIndex);
void executeProgram(char* name, int segment);
void terminate();
void handleInterrupt21(int ax, int bx, int cx, int dx);

int j = 0;

/* The main entry point of the program*/
int main ()
{
        /* todo
         * read a line from keyboard, maximum is 80 charaters
         * print this line to the screen
         */

        /* step 1: print to screen*/
        // printString ("Hello World\0");

        /* step 2: read from keyboard*/
        // char line[LENGTH_LINE];
        // printString ("Enter a line: \0");
        // readString (line);
        // printString (line);

        /* step 3: read a sector from disk*/
        // char buffer[LENGTH_SECTOR];
        // readSector (buffer, 30);
        // printString (buffer);

        /* step 4: creating my own interrupt
         * Link interrupt 21 to my service routine
         * In other words, set the interrupt table
         */
        // makeInterrupt21();
        // interrupt(0x21, 0, "Hello world\0", 0, 0);

         /* step 5: Load the directory sector 
          * into a 512 byte character array
          */
          
        /* 13312 = 26 * 512 represents the maximal storage is 26 sectors*/
        // char buffer[SIZE_BUFFER];
        // makeInterrupt21();

        /*read file into buffer*/
        // interrupt(0x21, 3, "messag\0", buffer, 0); 

        /*print out the file*/
        // interrupt(0x21, 0, buffer, 0, 0);

        /* step 6: load and execute*/
        makeInterrupt21();
        interrupt(0x21, 4, "shell\0", 0x2000, 0);

        /* step 7: terminate*/
        // makeInterrupt21();
        // interrupt(0x21, 5, 0, 0, 0);

        while (1)
        {
                // Infinite Loop
        }
        return 0;
}

void printString (char* word)
{
        /* Printing to Screen, Interrupt 0x10
         * C style string end with 0
         * if the character of word isn't 0 do loop
         * else return
         */

        /* start at a new line */
        char al = '\n';
        char ah = 0x0e;
        int ax = ah * 256 + al;
        interrupt(0x10, ax, 0, 0, 0);

        while (*word != '\0')
        {
                /* INT 10/ AH = 0Eh - video - teletype output */
                al = *word;
                ax = ah * 256 + al;
                interrupt (0x10, ax, 0, 0, 0);
                word++;
        }
        return;
}

void readString (char* line)
{
        /* read a line from keyboard input, inpterrupt 0x 16
         * end with the ENTER key whose ASCII is 0xd
         * get this line and store in the line buffer
         * deal with backspace and end characters
         */

        int i = 0;
        char temp = 0;
        while (1)
        {
        /* the AH must equal 0 and AL doesn't matter */
                temp = interrupt (0x16, 0, 0, 0, 0);
                
                /* only if the input is "ENTER", the loop ends*/
                if (temp == 0xd)
                        break;

                /* if input is backspace, only diplay not store and index minus one */
                if (temp == 0x8)
                {
                        if (i > 0)
                        {
                                interrupt (0x10, 0x0e * 256 + temp, 0, 0, 0);
                                i--;
                        }
                        else
                                continue;
                }

                /* store and display normal characters*/
                else
                {
                        line[i++] = temp;
                        interrupt (0x10, 0x0e * 256 + temp, 0, 0, 0);        
                }
        }

        /* remeber to set the last two characters as 0xa and 0x0 */
        if (i > LENGTH_LINE - 2)
                i = LENGTH_LINE - 2;
        line[i++] = 0xa;
        line[i] = '\0';
        return;
}

void readSector (char* buffer, int sector){
        /* read a sector from disk, interrupt 0x13
         * AH = 2 this number tells the BIOS to read
         * AL = number of sectors to read
         * BX = address where the data should be stored to
         * CH = track number
         * CL = relative sector number
         * DH = head number
         * DL = device number (for the floppy disk, use 0)
         */
        int ax = 2 * 256 + 1;
        int bx = buffer;
        int ch = div (sector, 36);
        int cl = mod (sector, 18) + 1;
        int cx = ch * 256 + cl;
        int dx = mod (div (sector, 18), 2) * 256 + 0;

        interrupt (0x13, ax, bx, cx, dx);

        buffer[LENGTH_SECTOR - 1] = '\0';
}

int mod (int dividend, int divisor)
{
        while (dividend >= divisor)
                dividend = dividend - divisor;
        return dividend;
}

int div (int dividend, int divisor)
{
        int q = 0;
        while (q * divisor <= dividend)
                q = q + 1;
        return q-1;
}

void handleInterrupt21(int ax, int bx, int cx, int dx){
        /* User Mode: Application program, Wrapper Function
         * Kernel Mode: Trap handler (interrupt table), System call service routine
         * To conlude, the interrupt fucntion I defined is running in kernel mode
         * ax = 0: print string of bx
         * ax = 1: read string of bx
         * ax = 2: read a sector from disc
         * ax = 3: read file into buffer
         */

        if (ax == 0)
                printString(bx);
        else 
            if (ax == 1)
                readString(bx);
        else 
            if (ax == 2)
                readSector(bx, cx);
        else 
            if (ax == 3)
                readFile(bx, cx); 
        else 
            if (ax == 4)
                executeProgram(bx, cx);
        else 
            if (ax == 5)
                terminate();
        else
                printString("Interrupt21 error!\r\n\0");
}

int matchFileName(char* buf, char* filename, int bufIndex){
        /* if file name matched return else return 0 */
        int i;
        
        for(i = 0; i < LENGTH_FILE_NAME; i++, bufIndex++){
                if(buf[bufIndex] != filename[i])
                        return 0;
        }
        return 1;
}

void readFile(char* fileName, char* buffer)
{
        /* go through the directory trying to match the file name
         * if not find it, return.
         */

        /* read a sector from disk */
        int i, j, sectorIndex;
        char tempBuf[LENGTH_SECTOR];
                
        for(i = 0; i < NUM_SECTORS; i++)
        {
                readSector(tempBuf, i);

                /* try to find the file name in a specific sector */
                for(j = 0; j < LENGTH_SECTOR; j++)
                        if((tempBuf[j] == fileName[0]) && (matchFileName(tempBuf, fileName, j) == 1))
                                break;

                /* if it is a match break, break loop*/
                if(j != LENGTH_SECTOR)
                        break;
        }

        if(i == NUM_SECTORS)
                return;

        /* Using the sector numbers in the directory, calling
         * "readSector"load the file, sector by sector, into 
         * the buffer array.
         */
         sectorIndex = (int) tempBuf[j + LENGTH_FILE_NAME];

         readSector(buffer, sectorIndex);         
         while(buffer[LENGTH_SECTOR - 1] != 0 && sectorIndex < NUM_SECTORS - 1)
         {
                         buffer = buffer + 512;
                         sectorIndex ++;
                         readSector(buffer, sectorIndex);         
         }         
}

void executeProgram(char* name, int segment)
{
    /* Call readFile to load the file into a buffer */
    int i;
    char buf[SIZE_BUFFER];

    readFile(name, buf);

    /* transfer the file from the buffer into the
     * bottom (0000) of memory at the segment in the 
     * parameter
     */
     for (i = 0; buf[i] != 0xff && i < SIZE_BUFFER; i++)
        putInMemory(segment, 0x0000 + i, buf[i]);

    /* Call the assembly function launchProgram()*/ 
    launchProgram(segment);
}

void terminate(){
    interrupt(0x21, 4, "shell\0", 0x2000, 0);
}