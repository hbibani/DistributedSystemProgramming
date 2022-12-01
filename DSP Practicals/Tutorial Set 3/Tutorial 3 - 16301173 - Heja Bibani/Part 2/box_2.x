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

struct mail_dims
{
	float volume;
	float mass;
};


program RPC_BOX
{
	/* define version 2 of our remote program and its procedures */

	version BOXVERSION2
	{
		box_results BOX_CALC(dimensions) = 1;
		float MAIL_CALC(mail_dims) = 2;
	} = 2;
} = 0x40049453;

