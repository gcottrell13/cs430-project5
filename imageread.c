
typedef struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} Pixel;

typedef struct {
	int width;
	int height;
	int max;
	int type;
	int valid;
	Pixel* data;
} PPMmeta;

PPMmeta CheckValidPPM(FILE *file)
{
	PPMmeta meta;
	
	meta.valid = 0;
	
	// these markers have values of:
	// 0 for not started
	// 1 for in progress
	// 2 for finished
	// together they make up a finite state machine
	
	int Mcomment = 0;
	int Mtype = 0;
	int Mwidth = 0;
	int Mheight = 0;
	int Mmax = 0;
	
	int buffer_len = 0;
	char* buffer = malloc(sizeof(char) * 4);
	
	while(1)
	{
		char c = fgetc(file);
		//printf("READ: %c\n", c);
		if(c == EOF)
		{
			break;
		}
		
		if(Mcomment == 1)
		{
			if(c == 10) // newline
			{
				Mcomment = 0;
			}
		}
		else
		{
			//printf("no comment\n");
			if(Mtype <= 1)
			{
				Mtype = 1;
				if(c == 'P')
				{
					int c2 = fgetc(file);
					char _type[2];
					_type[0] = c2;
					meta.type = atoi(_type);
					Mtype = 2;
					c2 = fgetc(file);
				}
				else
				{
					fprintf(stderr, "Absence of P in file header! %c\n", c);
					meta.valid = 1;
					return meta;
				}
			}
			else if(Mwidth <= 1)
			{
				Mwidth = 1;
				//printf("mwidth\n");
				
				if(c == '#')
				{
					Mcomment = 1;
				}
				else if(((c == 10 || c == ' ') && buffer_len > 0) || buffer_len >= 5)
				{
					Mwidth = 2; // done parsing width, put it into meta
					// we want an image with a positive width
					int width_value = atoi(buffer);
					if(width_value < 1) 
					{
						fprintf(stderr, "Image must have positive width: %d %s!\n", width_value, buffer);
						meta.valid = 1;
					}
					
					meta.width = width_value;
					buffer = malloc(sizeof(char) * 5);
					buffer_len = 0;
				}
				else
				{
					//printf("reading char to width: %c\n", c);
					buffer[buffer_len] = c;
					buffer_len ++;
				}
			}	
			else if(Mheight <= 1)
			{
				Mheight = 1;
				//printf("mheight\n");
				
				if(c == '#')
				{
					Mcomment = 1;
				}
				else if(((c == 10 || c == ' ') && buffer_len > 0) || buffer_len >= 5)
				{
					Mheight = 2; // done parsing height, put it into meta
					
					// we want an image with a positive height
					int height_value = atoi(buffer);
					if(height_value < 1) 
					{
						fprintf(stderr, "Image must have positive height!\n");
						meta.valid = 1;
					}
					
					meta.height = height_value;
					buffer = malloc(sizeof(char) * 5);
					buffer_len = 0;
				}
				else
				{
					//printf("reading char to height: %c\n", c);
					buffer[buffer_len] = c;
					buffer_len ++;
				}
			}	
			else if(Mmax <= 1)
			{
				Mmax = 1;
				
				if(c == ' ' || c == '#')
				{
					Mcomment = 1;
				}
				else if(c == 10 || buffer_len >= 5)
				{
					Mmax = 2; // done parsing max, put it into meta
					
					// the color channel should be 8 bits maximum
					int max_value = atoi(buffer);
					if(max_value > 255) 
					{
						fprintf(stderr, "Image must have 8 bit maximum color channels!\n");
						meta.valid = 1;
					}
					
					meta.max = max_value;
					break; // we're done here
				}
				else
				{
					//printf("reading char to max: %c\n", c);
					buffer[buffer_len] = c;
					buffer_len ++;
				}
			}
		}
		//printf("State of reading: type %d | width %d | height %d | max %d\n", Mtype, Mwidth, Mheight, Mmax);
	}
	
	return meta;
}


PPMmeta LoadPPM(char *filename)
{
	FILE *file = fopen(filename, "r");
	
	if(!file)
	{
		// stderr("source file does not exist! %s", src_name)
		fprintf(stderr, "Source file does not exist!\n");
		exit(1);
	}
	
	PPMmeta meta = CheckValidPPM(file);
	
	if(meta.valid == 1)
	{
		// write to stderr "invalid PPM file!"
		fprintf(stderr, "Invalid PPM file!\n");
		exit(1);
	}
	
	int size = meta.width * meta.height;
	
	Pixel* buffer = malloc(sizeof(Pixel) * size + 1);
	
	if(meta.type == 3)
	{
		char* cbuffer;
		int cbuffer_size;
		
		char character; // holds a character that was read in
		int c; // hold the index of the pixel
		
		for(c = 0; c < size; c++)
		{
			int r = -1;
			int g = -1;
			int b = -1;
			
			while(r < 0 || g < 0 || b < 0) // read values for each of the rgb components for a single pixel
			{
				cbuffer = malloc(sizeof(char) * 4); // set up a buffer
				cbuffer_size = 0; // the length of the number
				character = fgetc(file);
				while(character != 10 && cbuffer_size < 3)
				{
					cbuffer[cbuffer_size] = character; // store the character
					cbuffer_size ++; // increase the size of the number
					character = fgetc(file);
				}
				
				if(cbuffer_size > 0)
				{
					int value = atoi(cbuffer);
					if(r == -1) r = value;
					else if(g == -1) g = value;
					else if(b == -1) b = value;
				}
			}
			
			Pixel p;
			p.r = r;
			p.g = g;
			p.b = b;
			
			buffer[c] = p;
		}
	}
	else if(meta.type == 6)
	{
		//printf("type: 6, size: %d\n", size);
		int c;
		for(c = 0; c < size; c++)
		{
			Pixel* pixel = malloc(sizeof(Pixel));
			fread(pixel, sizeof(Pixel), 1, file);
			buffer[c] = *pixel;
		}
		
	}
	
	meta.data = buffer;
	
	return meta;
}

char* intToStr(int i, int size)
{
	char* str = malloc(sizeof(char) * size);
	sprintf(str, "%d", i);
	return str;
}

