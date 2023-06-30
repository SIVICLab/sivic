/***********************************************************************

  csi_linearize - bruker csi data reordered, e.g. spiral => linear
              (use as template for accessing and reordering bruker int fid data


 Details: spiral data reordered for 2dcsi processing
          pe1[] and pe2[] are used as acquisition order of pe gradients
          valid pe steps, 8x8 or 16x16

          To compile: cc -o csi_linearize csi_linearize.c
          multi-csi datasets not supported

          fid is bruker FID data; out-file must be named ser for sivic or jmrui
          
 Usage: csi_linearize  -n slices -r complex_points -p size -s size -i in_file -o out_file 
       -n must be set to 1 for a single 2dcsi dataset; 

         /data/pca4/600WB/programs/csi_linearize -n 1 -r 128 -p 16 -s 16 -i fid -o ser

 Input: bruker data - int fid  (note, sizeof data 4 bytes, i.e. 32bit int)
 
 Output: 2*rsize*psize*size csi2d fid file 

 Version: 20171127


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
    int		*p1;
    int        *out,*outi,*outx,*outy;
    int		r,p,s,t,u,v,d;  
    long int    i;    //note long int
    int         malloc_size_out,malloc_size;
    
   //pe12_8_list is not used
   int pe12_8_list[64] =
   {42,    41,    40,    39,    38,    37,    36,    63,
   43,    20,    19,    18,    17,    16,    35,    62,
   44,    21,     6,     5,     4,    15,    34,    61,
   45,    22,     7,     0,     3,    14,    33,    60,
   46,    23,     8,     1,     2,    13,    32,    59, 
   47,    24,     9,    10,    11,    12,    31,    58,  
   48,    25,    26,    27,    28,    29,    30,    57,   
   49,    50,    51,    52,    53,    54,    55,    56};    
    //pe12_list not used
    int pe12_list[256] = {
   210,   209,   208,   207,   206,   205,   204,   203,   202,   201,   200,   199,   198,   197,   196,   255,
   211,   156,   155,   154,   153,   152,   151,   150,   149,   148,   147,   146,   145,   144,   195,   254,
   212,   157,   110,   109,   108,   107,   106,   105,   104,   103,   102,   101,   100,   143,   194,   253,
   213,   158,   111,    72,    71,    70,    69,    68,    67,    66,    65,    64,    99,   142,   193,   252,
   214,   159,   112,    73,    42,    41,    40,    39,    38,    37,    36,    63,    98,   141,   192,   251,
   215,   160,   113,    74,    43,    20,    19,    18,    17,    16,    35,    62,    97,   140,   191,   250,
   216,   161,   114,    75,    44,    21,     6,     5,     4,    15,    34,    61,    96,   139,   190,   249,
   217,   162,   115,    76,    45,    22,     7,     0,     3,    14,    33,    60,    95,   138,   189,   248,
   218,   163,   116,    77,    46,    23,     8,     1,     2,    13,    32,    59,    94,   137,   188,   247,
   219,   164,   117,    78,    47,    24,     9,    10,    11,    12,    31,    58,    93,   136,   187,   246,
   220,   165,   118,    79,    48,    25,    26,    27,    28,    29,    30,    57,    92,   135,   186,   245,
   221,   166,   119,    80,    49,    50,    51,    52,    53,    54,    55,    56,    91,   134,   185,   244,
   222,   167,   120,    81,    82,    83,    84,    85,    86,    87,    88,    89,    90,   133,   184,   243,
   223,   168,   121,   122,   123,   124,   125,   126,   127,   128,   129,   130,   131,   132,   183,   242,
   224,   169,   170,   171,   172,   173,   174,   175,   176,   177,   178,   179,   180,   181,   182,   241,
   225,   226,   227,   228,   229,   230,   231,   232,   233,   234,   235,   236,   237,   238,   239,   240};

   int pe1[] = {
  0,-1,-1,0,   0,-1,-2,-2,   -2,-2,-1,0,   1,1,1,1,     1,0,-1,-2, -3,-3,-3,-3,  -3,-3,-2,-1,   0,1,2,2,  //0-31
   2,2,2,2,     2,1,0,-1,     -2,-3,-4,-4, -4,-4,-4,-4, -4,-4,-3,-2, -1,0,1,2,    3,3,3,3,       3,3,3,3,  //32-63
   3,2,1,0,     -1,-2,-3,-4,  -5,-5,-5,-5, -5,-5,-5,-5, -5,-5,-4,-3, -2,-1,0,1,   2,3,4,4,       4,4,4,4,  //64-95
   4,4,4,4,     4,3,2,1,      0,-1,-2,-3,  -4,-5,-6,-6, -6,-6,-6,-6, -6,-6,-6,-6, -6,-6,-5,-4,  -3,-2,-1,0,  //96-127
   1,2,3,4,     5,5,5,5,      5,5,5,5,     5,5,5,5,      5,4,3,2,     1,0,-1,-2,   -3,-4,-5,-6, -7,-7,-7,-7, //128-159
   -7,-7,-7,-7, -7,-7,-7,-7, -7,-7,-6,-5, -4,-3,-2,-1,  0,1,2,3,     4,5,6,6,      6,6,6,6,      6,6,6,6, //160-191
   6,6,6,6,     6,5,4,3,      2,1,0,-1,    -2,-3,-4,-5, -6,-7,-8,-8, -8,-8,-8,-8, -8,-8,-8,-8,  -8,-8,-8,-8,//192-223
   -8,-8,-7,-6, -5,-4,-3,-2, -1,0,1,2,     3,4,5,6,     7,7,7,7,     7,7,7,7,      7,7,7,7,      7,7,7,7};    //224-255
                           
   int pe2[] = {
   0,-1,0,-1,   -2,-2,-2,-1,  0,1,1,1,      1,0,-1,-2,  -3,-3,-3,-3,  -3,-2,-1,0,  1,2,2,2,     2,2,2,1,   //0-31
   0,-1,-2,-3,  -4,-4,-4,-4, -4,-4,-4,-3,  -2,-1,0,1,   2,3,3,3,      3,3,3,3,     3,2,1,0,    -1,-2,-3,-4, //32-63
   -5,-5,-5,-5, -5,-5,-5,-5, -5,-4,-3,-2,  -1,0,1,2,    3,4,4,4,      4,4,4,4,     4,4,4,3,     2,1,0,-1,  //64-95
   -2,-3,-4,-5, -6,-6,-6,-6, -6,-6,-6,-6,  -6,-6,-6,-5, -4,-3,-2,-1,  0,1,2,3,     4,5,5,5,     5,5,5,5,   //96-127
   5,5,5,5,      5,4,3,2,     1,0,-1,-2,   -3,-4,-5,-6, -7,-7,-7,-7, -7,-7,-7,-7,  -7,-7,-7,-7, -7,-6,-5,-4, //128-159
   -3,-2,-1,0,   1,2,3,4,     5,6,6,6,      6,6,6,6,     6,6,6,6,     6,6,6,5,      4,3,2,1,     0,-1,-2,-3,  //160-191
   -4,-5,-6,-7, -8,-8,-8,-8, -8,-8,-8,-8,  -8,-8,-8,-8, -8,-8,-8,-7, -6,-5,-4,-3,  -2,-1,0,1,    2,3,4,5,    //192-223
   6,7,7,7,      7,7,7,7,     7,7,7,7,      7,7,7,7,     7,6,5,4,     3,2,1,0,     -1,-2,-3,-4, -5,-6,-7,-8}; //224-255
 
    /* Check arguments */

    if (argc-1 != NUM_ARGS)
    {
	printf("Usage: csi_linearize  -n slice_offset -r size -p size -s size -i in_file -o out_file \n");
        printf("Example: csi_linearize -n 1 -s 16 -p 16 -r 128 -i fid -o spectra/ser_1 \n");

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

        printf("csi_linearize: n r p s = %d %d %d %d \n",nr, rsize,psize,ssize);

    //check size limits
    if((rsize < 64) || (rsize > 512))
    {
        printf("csi_linearize: Illegal rsize = %d",rsize);
        exit(1);
    } 
    if((psize < 8) || (psize > 16))
    {
        printf("csi_linearize: Illegal psize = %d",psize);
        exit(1);
    }
    if((ssize < 8) || (ssize > 16))
    {
        printf("csi_linearize: Illegal ssize = %d",ssize);
        exit(1);
    }    
    if(psize != ssize) 
    {
      printf("csi_linearize: psize=%d not equal to ssize=%d",psize,ssize);
      exit(1);
    }
    totsize = 2*psize*rsize*ssize*nr;  //data size of multi-image input file

    /* open the field map phase files, frequency file, and parameter files */
	/* open input files */

        i = open(filename, O_RDONLY);
	infile = (FILE *) i;   //note long int
        if(i < 0)
	{
		printf("csi_linearize: cannot open input file #1\n");
		exit(1);
	}

	/* open input files ***This doesn't work !!!! *****
	if ((infile = open(filename, O_RDONLY)) < 0)
	{
		printf("gsvtobin: cannot open input file #1\n");
		exit(1);
	} **/
                /* note float size 32bits, same as int */
                malloc_size = sizeof(int);  /* re+im */
                malloc_size = totsize*sizeof(int);  /* re+im */

                malloc_size_out = 2*rsize*psize*ssize*sizeof(int);  //outpul slice
		if ((p1 = (int *)malloc(malloc_size)) == NULL)
		{
			printf("csi_linearize: unable to allocate space for input data file #1");
			exit(1);
		}	
		if (read(infile, p1, malloc_size) != malloc_size)
		{
			printf("csi_linearize: unable to read input data file #1\n");
			exit(1);
		}

		if ((out = (int *)malloc(malloc_size_out)) == NULL)
		{
			printf("csi_linearize: unable to allocate space for output data file #1");
			exit(1);
		}
		if ((outi = (int *)malloc(malloc_size_out)) == NULL)
		{
			printf("csi_linearize: unable to allocate space for output data file #1");
			exit(1);
		}
		if ((outx = (int *)malloc(malloc_size_out)) == NULL)
		{
			printf("csi_linearize: unable to allocate space for output data file #1");
			exit(1);
		}
		if ((outy = (int *)malloc(malloc_size_out)) == NULL)
		{
			printf("csi_linearize: unable to allocate space for output data file #1");
			exit(1);
		}


	i = open(outname, O_CREAT|O_TRUNC|O_WRONLY, 0644);
        outfile = (FILE *) i;
        if(i < 0)
	{
		printf("csi_linearize: cannot open output file \n");
		exit(1);
	}
/**
       //to extract a 2dcsi slice or time point
        p = 2*rsize*psize*ssize*(nr-1);  //note nr index 0 to images-1
        for(r=0; r<rsize*psize*ssize*2; r++) {
            out[r] = p1[p++];
        }
**/
         i = 0; t=0;
         for(d=0; d<(psize*ssize); d++) {
           p = pe1[d] + psize/2;  //PE step count: 0 to psize-1
           s = pe2[d] + ssize/2;  //0 to ssize-1
           for(r=0; r<(rsize*2); r++) {
             t= (s*psize*rsize*2) + (p*rsize*2) + r;
             out[t++] = p1[i++];  //write to intermed buffer
           }
         }
        
/**
       // order rows (pe1) into position along pe2 
        u = ssize/2; //out index to even pe2
        v = (ssize/2)-1; //out index to odd pe2
        for(s=0; s<ssize; s++) {
          t = u * psize*rsize*2; //out ptr for even pe2 
          for(p=0; p<psize; p++) {
            i= (s*psize*rsize*2) + (p*rsize*2);
            for(r=0; r<rsize*2; r++) {
              outi[t++] = p1[i++];  //write to intermed buffer
            }
          }
          u++;
          s++;
          t = v * psize*rsize*2; //out ptr for even pe2 
          for(p=0; p<psize; p++) {
            i= (s*psize*rsize*2) + (p*rsize*2);
            for(r=0; r<rsize*2; r++) {
              outi[t++] = p1[i++];
            }
          }
          v--;
        }  
**/
/**
       // reorder columns in the pe1 direction
        i = 0; //input index, 0=>totalsize-1 
        for(s=0; s<ssize; s++) {
          u = psize/2; //out index to even pe2
          v = (psize/2)-1; //out index to odd pe2
          for(p=0; p<psize; p++) {
            t = (s*psize*rsize*2) +(u*rsize*2); //out ptr for even pe2 
            for(r=0; r<rsize*2; r++) {
              outx[t++] = outi[i++];
            }
            p++;
            t = (s*psize*rsize*2) +(v*rsize*2); //out ptr for odd pe2 
            for(r=0; r<rsize*2; r++) {
              outx[t++] = outi[i++];
            }
            u++;
            v--;
          }
        }  
**/
/**
        //transpose data
        //assume ssize=psize
        i = 0;
        for(s=0; s<ssize; s++) {
          for(p=0; p<psize; p++) {
            t = (p*psize*rsize*2) + (s*rsize*2);
            for(r=0; r<rsize*2; r++) {
              outy[t++] = outx[i++];  
            }
          }
        }
**/
/**
        // flip along slice or pe2 direction, i.e. L-R
        t = 0; //counter 0 -> rsize*psize*ssize*2
        for(s=0; s<ssize; s=s+2) {  
          //swap odd and even pe2 step
          s=s+1; //odd pe2 
          for(p=0; p<psize; p++) {
            i= (s*psize*rsize*2) + (p*rsize*2);
            for(r=0; r<rsize*2; r++) {
              out[t++] = outy[i++];
            }
          }
          s=s-1;  //even pe2
          for(p=0; p<psize; p++) {
            i= (s*psize*rsize*2) + (p*rsize*2);
            for(r=0; r<rsize*2; r++) {
              out[t++] = outy[i++];
            }
          }
        }  
**/
/**
       //to flip along pe2 direction; ie up-down or  o
        t = 0; //counter 0 -> rsize*psize*ssize*2
        for(s=0; s<ssize; s++) {
          for(p=0; p<psize; p=p+2) {   //swap odd-even pairs
            p=p+1; 
            i= (s*psize*rsize*2) + (p*rsize*2);
            for(r=0; r<rsize*2; r++) {
              out[t++] = outy[i++];
            }
            p=p-1; 
            i= (s*psize*rsize*2) + (p*rsize*2);
            for(r=0; r<rsize*2; r++) {
              out[t++] = outy[i++];
            }
          }
        }  

**/
		/* write out the data output buffer */
		if (write(outfile, out, malloc_size_out) <0)
		{
			printf("csi_linearize: unable to write data \n");
			exit(1);
		}		
}

/*************************************************************************
		Modification History
		
20171212 Bug: 195th point error pe2=-8 => pe2=-7
20171217 element 3 of arrays pe1 and pe2 moved to [0]
**************************************************************************/
