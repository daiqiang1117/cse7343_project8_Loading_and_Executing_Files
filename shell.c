/*      shell.c
      	
        Qiang Dai
*/
#define TYPE_COMMAND "type\0"
#define EXECUTE_COMMAND "execute\0"
#define UNDEFINED_COMMAND "undefined\0"
#define LENGTH 80
#define SIZE_BUFFER 13312

void runCommand();
char* commanddType();
char* cmmd();
char* argument1();
void clearBuffer();
char* strStr(char* str, char ch);
int strcmp(char* str1, char* str2);

char buffer[LENGTH];

int main()
{
	while (1) 
	{
		buffer[0] = '\0';
		/* prompt user*/
		interrupt(0x21, 0, "\r\nQiangDai> \0", 0, 0);
		
		/* read command line string into buffer */
		interrupt(0x21, 1, buffer, 0, 0);
		
		/* running the command*/
		runCommand();

		/* clear the buffer */
		clearBuffer();
	}
	return 0;
}

/* running the commmand from shell according to the command type */
void runCommand()
{
	char buf[SIZE_BUFFER];
	char* commandType = commanddType();

	if (commandType == TYPE_COMMAND)
	{
		/* load file into memory */
		interrupt(0x21, 3, argument1(), buf, 0); 

		/* print out the content of the file*/
		interrupt(0x21, 0, buf, 0, 0); 	
	}
		
	if (commandType == EXECUTE_COMMAND)
	{
		/* execute the file*/
		interrupt(0x21, 4, argument1(), 0x2000, 0);

		/* call the terminate in kernel.c  to re-prompt the shell*/
		interrupt(0x21, 5, 0, 0, 0);
	}

	if (commandType == UNDEFINED_COMMAND)
		interrupt(0x21, 0, "command not found\r\n\0", 0, 0);
}

char* commanddType()
{
	/* get cmd sub-string */
	char* cmd = cmmd();

	/* check invalid cmd format */
	if(cmd == '\0')
	{
		interrupt(0x21, 0, "invalid command format\r\n\0", 0, 0);
		return '\0';
	}

	/* determine the commmand type*/
	if(strcmp(TYPE_COMMAND, cmd) == 1)
		return TYPE_COMMAND;
	if(strcmp(EXECUTE_COMMAND, cmd) == 1)
		return EXECUTE_COMMAND;
	return UNDEFINED_COMMAND;
}


/* get the command in the input string*/
char* cmmd()
{
	char* cmd = strStr(buffer, ' ');
	*cmd = '\0';
	return buffer;
}

/* get the first argument in the input string*/
char* argument1()
{
	char* arg = strStr(buffer, ' ');
	/* check invalid cmd format */
	if(arg == '\0')
	{
		interrupt(0x21, 0, "invalid command format\r\n\0", 0, 0);
		return '\0';
	}	
	else
	{
		arg++;
		return arg;
	}
}

void clearBuffer()
{
	int i;
	for (i = 0; i < LENGTH; i++)
		buffer[i] = '\0';
}

/* return 1 if they are same string else return 0 */
int strcmp(char* str1, char* str2)
{
	while((*str1 != '\0') && (*str2 != '\0' ))
	{
		if (*str1 != *str2)
			break;
		str1 ++;
		str2 ++;
	}
	if (*str1 == *str2)
		return 1;
	return 0;
}

/* return a substring which begin at the first appearence 
 * of a speciafic character; if matched return the substring
 * else return '\0'
 */
char* strStr(char* str, char ch)
{
	while(*str != '\0')
	{
		if(*str == ch)
			break;
		str++;
	}
	return str;
}

