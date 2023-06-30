/***********************************************************************

  split_csi - to split dynamic csi2d slice from fid data

 Details: bruker multi csi fid split into NR files for processing by sivic

          To compile: cc -o split_csi split_csi.c
          Note: slice dimension refers to nr 
          
Usage: split_csi  -n slice_offset -r size -p size -s size -i in_file -o out_file 

         /data/pca4/600WB/programs/split_csi


 
 Input: bruker data - int fid  (note, float used to define sizeof data, i.e. 32bit)
 
 Output: rsize*psize*size csi2d fid file 

 Version: 20170621

20171024: Read size limit 128=>64

***********************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/errno.h>
#include <sys/fcntl.h>

#define NUM_ARGS	12   /* number of input arguments */

/* I/O string */
char		s[80];

main(argc,argv)
int 	argc;
char	*argv[];
{
    FILE	*infile,*outfile;
    char	filename[80],outname[80];
    int		args;
    int		totsize,ssize,rsize,psize;
    int	        nr;	
    float	*p1;
    float	max;
    float       *out;
    int		r,p,s,t;  
    long int    i;    //note long int
    int         malloc_size_out,malloc_size;

    /* Check arguments */

    if (argc-1 != NUM_ARGS)
    {
	printf("Usage: split_csi  -n slice_offset -r size -p size -s size -i in_file -o out_file \n");
        printf("Example: split_csi -n 1 -s 10 -p 8 -r 8 -i fid -o spectra/ser_1 \n");

	exit(1);
    }

    /*
     * process arguments
     */

    args = 1;
    totsize = 0;

      while ( argv[args][0] == '-' ) {
	switch (argv[args++][1])
	{
	    case 'n':
		nr = atoi(argv[args++]);	//offset to slice     
		break;
	    case 's':	
		ssize = atoi(argv[args++]);
		break;
	    case 'p':	
		psize = atoi(argv[args++]);		
		break;
	    case 'r':	
		rsize = atoi(argv[args++]);		
		break;
            case 'i': //input file
                strcpy(filename, argv[args++]);
                break;
            case 'o': //output file
                strcpy(outname, argv[args++]);
                break;
	}
        if(args > NUM_ARGS)
          args--;  //to avoid segmentation error
      }


    //check size limits
    if((rsize < 64) || (rsize > 512))
    {
        printf("split_csi: Illegal rsize = %d",rsize);
        exit(1);
    } 
    if((psize < 8) || (psize > 32))
    {
        printf("split_csi: Illegal psize = %d",psize);
        exit(1);
    }
    if((ssize < 8) || (ssize > 32))
    {
        printf("split_csi: Illegal ssize = %d",ssize);
        exit(1);
    }    
    totsize = 2*psize*rsize*ssize*nr;  //data size of multi-image input file

    /* open the field map phase files, frequency file, and parameter files */
	/* open input files */

        i = open(filename, O_RDONLY);
	infile = (FILE *) i;   //note long int
        if(i < 0)
	{
		printf("split_csi: cannot open input file #1\n");
		exit(1);
	}

	/* open input files ***This doesn't work !!!! *****
	if ((infile = open(filename, O_RDONLY)) < 0)
	{
		printf("gsvtobin: cannot open input file #1\n");
		exit(1);
	} **/
                /* note float size 32bits, same as int */
                malloc_size = totsize*sizeof(float);  /* re+im */
                malloc_size_out = 2*rsize*psize*ssize*sizeof(float);  //outpul slice
		if ((p1 = (float *)malloc(malloc_size)) == NULL)
		{
			printf("split_csi: unable to allocate space for input data file #1");
			exit(1);
		}	
		if (read(infile, p1, malloc_size) != malloc_size)
		{
			printf("split_csi: unable to read input data file #1\n");
			exit(1);
		}

		if ((out = (float *)malloc(malloc_size_out)) == NULL)
		{
			printf("split_csi: unable to allocate space for output data file #1");
			exit(1);
		}


	i = open(outname, O_CREAT|O_TRUNC|O_WRONLY, 0644);
        outfile = (FILE *) i;
        if(i < 0)
	{
		printf("split_csi: cannot open output file \n");
		exit(1);
	}
        p = 2*rsize*psize*ssize*(nr-1);  //note nr index 0 to images-1
        for(r=0; r<rsize*psize*ssize*2; r++) {
            out[r] = p1[p++];
        }


		/* write out the data output buffer */
		if (write(outfile, out, malloc_size_out) <0)
		{
			printf("split_csi: unable to write data \n");
			exit(1);
		}		
}

/*************************************************************************
		Modification History
		
**************************************************************************/
