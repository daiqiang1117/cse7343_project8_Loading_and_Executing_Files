/*      shell.c
      	
        Qiang Dai
*/

int main()
{
	char buffer[80];

	while (1) 
	{
		interrupt(0x21, 0, "\r\nQiangDai> \0", 0, 0);
		interrupt(0x21, 1, buffer, 0, 0);
		// parse the contents of the buffer
		interrupt(0x21, 0, "command not found\r\n\0", 0, 0);
		// clear the buffer
	}

	return 0;
}