/* box_main.c
does calculations about boxes using remote procedure calls
this is the client code
compile it with the rpcgened code
cc box_main.c box_clnt.c box_xdr.c -o box_client
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"Box.h"

/* print out the box details */
void print_box(float l, float w, float h, float sa, float v)
{
	printf("A box of dimensions %f x %f x %f\n", l, w, h);
	printf("Has a surface area of %f\n", sa);
	printf("and a volume of %f\n", v);
	return;
}

int main(int ac, char** av)
{
	CLIENT* c_handle; /*client handle*/
	dimensions dims;  /*dimesnions structure specified in box.h*/
	box_results* res; /*results structure recieved from server specified in box.h*/

	/* everything comes off the command line so check its there */
	if (ac != 5)
	{
		fprintf(stderr, "Usage: %s remote_host length width height\n", av[0]);
		exit(EXIT_FAILURE);
	}

	/* grab the dims and shove them in the struct */
	dims.length = atof(av[2]);
	dims.width = atof(av[3]);
	dims.height = atof(av[4]);

	/*
	   create a connection handle to make rpc connection
	   use localhost parameter as identifer of server location
	   and program and its version, specify mechanism of
	   protocol [udp]
	*/
	c_handle = clnt_create(av[1], RPC_BOX, BOXVERSION1, "udp");

	if (c_handle == NULL)
	{
		clnt_pcreateerror(av[1]);
		exit(EXIT_FAILURE);
	}

	/* make the rpc */
	res = box_calc_1(&dims, c_handle);

	if (res == NULL)
	{
		clnt_perror(c_handle, av[1]);
		exit(EXIT_FAILURE);
	}

	/* print out the results */
	print_box(dims.length, dims.width, dims.height, res->surface, res->volume);

	/* free up the RPC handle */
	clnt_destroy(c_handle);
	return 0;
}
