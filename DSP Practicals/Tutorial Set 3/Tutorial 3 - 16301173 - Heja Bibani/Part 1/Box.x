/* Protocol Definition file: box.x */
/* the input box dimensions */
struct dimensions
{
	float length;
	float width;
	float height;
};

/* returned results */
struct box_results
{
	float volume;
	float surface;
};

program RPC_BOX
{
	/* define version 1 of our remote program and its procedures */
	version BOXVERSION1
	{
	box_results BOX_CALC(dimensions) = 1;
	} = 1;
} = 0x40049453;

/*
Note the program number is in the temporary range 0x40000000 - 0x5FFFFFFF
The other ranges are:
0x00000000 - 0x1FFFFFFF Administered by Sun Mircrosystems
0x20000000 - 0x3FFFFFFF For site managers
0x40000000 - 0x5FFFFFFF Transient uses
0x60000000 - 0xFFFFFFFF Reserved for future use
*/
