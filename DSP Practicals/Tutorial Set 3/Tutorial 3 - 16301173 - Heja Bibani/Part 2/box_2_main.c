/* box_main.c
	does calculations about boxes and prices using remote procedure calls
	cc box_main.c box_clnt.c box_xdr.c -o box_client
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"box_2.h"

/* print out the box details */
void print_box(float l, float w, float h, float sa, float v)
{
	printf("A box of dimensions %f x %f x %f\n", l, w, h);
	printf("Has a surface area of %f\n", sa);
	printf("and a volume of %f\n", v);
	return;
}

/*print price and other details*/
void print_price(float mass, float volume, float price)
{
	printf("A box with a volume of %f and mass of %f\nCosts: ", volume, mass);
	printf("$%f\n", price);
	return;
}

int main(int ac, char** av)
{
	CLIENT* c_handle;   /*client handle*/
	dimensions dims;    /*dimesnions structure specified in box.h*/
	mail_dims maildims; /*dimesnions of volume and mass structure*/
	box_results* res;   /*results box structure recieved from server specified in box.h*/
	float* price_res;   /*result float price recieved from server specified in box.h*/

	/* everything comes off the command line so check its there */
	if (ac != 6)
	{
		fprintf(stderr, "Usage: %s remote_host length width height mass\n", av[0]);
		exit(EXIT_FAILURE);
	}

	/* grab the dims and shove them in the struct */
	dims.length = atof(av[2]);
	dims.width = atof(av[3]);
	dims.height = atof(av[4]);
	maildims.mass = atof(av[5]);

	/*
	   create a connection handle to make rpc connection
	   use localhost parameter as identifer of server location
	   and program and its version, specify mechanism of
	   protocol [udp]
	*/
	c_handle = clnt_create(av[1], RPC_BOX, BOXVERSION2, "udp");

	if (c_handle == NULL)
	{
		clnt_pcreateerror(av[1]);
		exit(EXIT_FAILURE);
	}

	/* make the rpc */
	res = box_calc_2(&dims, c_handle);			/*call remote procedure box_calc_2*/

	if (res == NULL)
	{
		clnt_perror(c_handle, av[1]);
		exit(EXIT_FAILURE);
	}

	maildims.volume = res->volume;      			/*get volume results*/
	price_res = mail_calc_2(&maildims, c_handle); /*call remote procedure mail_calc_2*/

	if (price_res == NULL)
	{
		clnt_perror(c_handle, av[1]);
		exit(EXIT_FAILURE);
	}

	/* print out the results */
	print_box(dims.length, dims.width, dims.height, res->surface, res->volume);
	print_price(maildims.mass, maildims.volume, *price_res);
	/* free up the RPC handle */
	clnt_destroy(c_handle);
	return 0;
}
