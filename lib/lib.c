
void memset(char *src, unsigned int num, unsigned int len)
{
	int i = 0;
	for (i = 0; i < len; i++)
	{
		*src++ = num;
	}
}
