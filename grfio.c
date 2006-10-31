
//----------------------------
//	grf decode data table
//----------------------------
static unsigned char BitMaskTable[8] = {
	0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};

static char BitSwapTable1[64] = {
	58, 50, 42, 34, 26, 18, 10,  2, 60, 52, 44, 36, 28, 20, 12,  4,
	62, 54, 46, 38, 30, 22, 14,  6, 64, 56, 48, 40, 32, 24, 16,  8,
	57, 49, 41, 33, 25, 17,  9,  1, 59, 51, 43, 35, 27, 19, 11,  3,
	61, 53, 45, 37, 29, 21, 13,  5, 63, 55, 47, 39, 31, 23, 15,  7
};
static char	BitSwapTable2[64] = {
	40,  8, 48, 16, 56, 24, 64, 32, 39,  7, 47, 15, 55, 23, 63, 31,
	38,  6, 46, 14, 54, 22, 62, 30, 37,  5, 45, 13, 53, 21, 61, 29,
	36,  4, 44, 12, 52, 20, 60, 28, 35,  3, 43, 11, 51, 19, 59, 27,
	34,  2, 42, 10, 50, 18, 58, 26, 33,  1, 41,  9, 49, 17, 57, 25
};
static char	BitSwapTable3[32] = {
	16,  7, 20, 21, 29, 12, 28, 17,  1, 15, 23, 26,  5, 18, 31, 10,
     2,  8, 24, 14, 32, 27,  3,  9, 19, 13, 30,  6, 22, 11,  4, 25
};

static unsigned char NibbleData[4][64]={
	{
		0xef, 0x03, 0x41, 0xfd, 0xd8, 0x74, 0x1e, 0x47,  0x26, 0xef, 0xfb, 0x22, 0xb3, 0xd8, 0x84, 0x1e,
		0x39, 0xac, 0xa7, 0x60, 0x62, 0xc1, 0xcd, 0xba,  0x5c, 0x96, 0x90, 0x59, 0x05, 0x3b, 0x7a, 0x85,
		0x40, 0xfd, 0x1e, 0xc8, 0xe7, 0x8a, 0x8b, 0x21,  0xda, 0x43, 0x64, 0x9f, 0x2d, 0x14, 0xb1, 0x72,
		0xf5, 0x5b, 0xc8, 0xb6, 0x9c, 0x37, 0x76, 0xec,  0x39, 0xa0, 0xa3, 0x05, 0x52, 0x6e, 0x0f, 0xd9,
	}, {
		0xa7, 0xdd, 0x0d, 0x78, 0x9e, 0x0b, 0xe3, 0x95,  0x60, 0x36, 0x36, 0x4f, 0xf9, 0x60, 0x5a, 0xa3,
		0x11, 0x24, 0xd2, 0x87, 0xc8, 0x52, 0x75, 0xec,  0xbb, 0xc1, 0x4c, 0xba, 0x24, 0xfe, 0x8f, 0x19,
		0xda, 0x13, 0x66, 0xaf, 0x49, 0xd0, 0x90, 0x06,  0x8c, 0x6a, 0xfb, 0x91, 0x37, 0x8d, 0x0d, 0x78,
		0xbf, 0x49, 0x11, 0xf4, 0x23, 0xe5, 0xce, 0x3b,  0x55, 0xbc, 0xa2, 0x57, 0xe8, 0x22, 0x74, 0xce,
	}, {
		0x2c, 0xea, 0xc1, 0xbf, 0x4a, 0x24, 0x1f, 0xc2,  0x79, 0x47, 0xa2, 0x7c, 0xb6, 0xd9, 0x68, 0x15,
		0x80, 0x56, 0x5d, 0x01, 0x33, 0xfd, 0xf4, 0xae,  0xde, 0x30, 0x07, 0x9b, 0xe5, 0x83, 0x9b, 0x68,
		0x49, 0xb4, 0x2e, 0x83, 0x1f, 0xc2, 0xb5, 0x7c,  0xa2, 0x19, 0xd8, 0xe5, 0x7c, 0x2f, 0x83, 0xda,
		0xf7, 0x6b, 0x90, 0xfe, 0xc4, 0x01, 0x5a, 0x97,  0x61, 0xa6, 0x3d, 0x40, 0x0b, 0x58, 0xe6, 0x3d,
	}, {
		0x4d, 0xd1, 0xb2, 0x0f, 0x28, 0xbd, 0xe4, 0x78,  0xf6, 0x4a, 0x0f, 0x93, 0x8b, 0x17, 0xd1, 0xa4,
		0x3a, 0xec, 0xc9, 0x35, 0x93, 0x56, 0x7e, 0xcb,  0x55, 0x20, 0xa0, 0xfe, 0x6c, 0x89, 0x17, 0x62,
		0x17, 0x62, 0x4b, 0xb1, 0xb4, 0xde, 0xd1, 0x87,  0xc9, 0x14, 0x3c, 0x4a, 0x7e, 0xa8, 0xe2, 0x7d,
		0xa0, 0x9f, 0xf6, 0x5c, 0x6a, 0x09, 0x8d, 0xf0,  0x0f, 0xe3, 0x53, 0x25, 0x95, 0x36, 0x28, 0xcb,
	}
};

/*==========================================
 *	Grf data decode : Subs
 *------------------------------------------
 */
static void NibbleSwap(BYTE *Src, int len) {
	for( ; 0 < len; len--, Src++) {
		*Src = (*Src >> 4) | (*Src << 4);
	}
}

static void BitConvert(BYTE *Src, char *BitSwapTable) {
	int lop, prm;
	BYTE tmp[8];
	*(DWORD*)tmp = *(DWORD*)(tmp+4)=0;

	for(lop=0;lop!=64;lop++) {
		prm = BitSwapTable[lop]-1;
		if (Src[(prm >> 3) & 7] & BitMaskTable[prm & 7]) {
			tmp[(lop >> 3) & 7] |= BitMaskTable[lop & 7];
		}
	}
	*(DWORD*)Src     = *(DWORD*)tmp;
	*(DWORD*)(Src+4) = *(DWORD*)(tmp+4);
}

static void BitConvert4(BYTE *Src) {
	int lop,prm;
	BYTE tmp[8];

	tmp[0] = ((Src[7]<<5) | (Src[4]>>3)) & 0x3f;	// ..0 vutsr
	tmp[1] = ((Src[4]<<1) | (Src[5]>>7)) & 0x3f;	// ..srqpo n
	tmp[2] = ((Src[4]<<5) | (Src[5]>>3)) & 0x3f;	// ..o nmlkj
	tmp[3] = ((Src[5]<<1) | (Src[6]>>7)) & 0x3f;	// ..kjihg f
	tmp[4] = ((Src[5]<<5) | (Src[6]>>3)) & 0x3f;	// ..g fedcb
	tmp[5] = ((Src[6]<<1) | (Src[7]>>7)) & 0x3f;	// ..cba98 7
	tmp[6] = ((Src[6]<<5) | (Src[7]>>3)) & 0x3f;	// ..8 76543
	tmp[7] = ((Src[7]<<1) | (Src[4]>>7)) & 0x3f;	// ..43210 v

	for(lop=0;lop!=4;lop++) {
		tmp[lop] = (NibbleData[lop][tmp[lop*2]] & 0xf0)
		         | (NibbleData[lop][tmp[lop*2+1]] & 0x0f);
	}

	*(DWORD*)(tmp+4)=0;
	for(lop=0;lop!=32;lop++) {
		prm = BitSwapTable3[lop]-1;
		if (tmp[prm >> 3] & BitMaskTable[prm & 7]) {
			tmp[(lop >> 3) + 4] |= BitMaskTable[lop & 7];
		}
	}
	*(DWORD*)Src ^= *(DWORD*)(tmp+4);
}

static void decode_des_etc(BYTE *buf, int len, int type, int cycle) {
	int lop, cnt = 0;

	if (cycle<3) cycle=3;
	else if (cycle<5) cycle++;
	else if (cycle<7) cycle+=9;
	else cycle+=15;

	for(lop=0;lop*8<len;lop++,buf+=8) {
		if (lop < 20 || (type == 0 && lop % cycle == 0)) { // des
			BitConvert(buf, BitSwapTable1);
			BitConvert4(buf);
			BitConvert(buf, BitSwapTable2);
		} else {
			if (cnt == 7 && type == 0) {
				int a;
				BYTE tmp[8];
				*(DWORD*)tmp     = *(DWORD*)buf;
				*(DWORD*)(tmp+4) = *(DWORD*)(buf+4);
				cnt=0;
				buf[0]=tmp[3];
				buf[1]=tmp[4];
				buf[2]=tmp[6];
				buf[3]=tmp[0];
				buf[4]=tmp[1];
				buf[5]=tmp[2];
				buf[6]=tmp[5];
				a=tmp[7];
				if (a==0x00) a=0x2b;
				else if (a==0x2b) a=0x00;
				else if (a==0x01) a=0x68;
				else if (a==0x68) a=0x01;
				else if (a==0x48) a=0x77;
				else if (a==0x77) a=0x48;
				else if (a==0x60) a=0xff;
				else if (a==0xff) a=0x60;
				else if (a==0x6c) a=0x80;
				else if (a==0x80) a=0x6c;
				else if (a==0xb9) a=0xc0;
				else if (a==0xc0) a=0xb9;
				else if (a==0xeb) a=0xfe;
				else if (a==0xfe) a=0xeb;
				buf[7] = a;
			}
			cnt++;
		}
	}
}
/*==========================================
 *	Grf data decode sub : zip
 *------------------------------------------
 */
static int decode_zip(Bytef* dest, uLongf* destLen, const Bytef* source, uLong sourceLen) {
	z_stream stream;
	int err;

	stream.next_in = (Bytef*)source;
	stream.avail_in = (uInt)sourceLen;
	/* Check for source > 64K on 16-bit machine: */
	if ((uLong)stream.avail_in != sourceLen)
		return Z_BUF_ERROR;

	stream.next_out = dest;
	stream.avail_out = (uInt)*destLen;
	if ((uLong)stream.avail_out != *destLen)
		return Z_BUF_ERROR;

	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;

	err = inflateInit(&stream);
	if (err != Z_OK) return err;

	err = inflate(&stream, Z_FINISH);
	if (err != Z_STREAM_END) {
		inflateEnd(&stream);
		return err == Z_OK ? Z_BUF_ERROR : err;
	}
	*destLen = stream.total_out;

	err = inflateEnd(&stream);

	return err;
}

static unsigned char * decode_filename(unsigned char *buf, int len) {
	int lop;

	for(lop=0;lop<len;lop+=8) {
		NibbleSwap(&buf[lop],8);
		BitConvert(&buf[lop],BitSwapTable1);
		BitConvert4(&buf[lop]);
		BitConvert(&buf[lop],BitSwapTable2);
	}

	return buf;
}

/*==========================================
 * Grfio : Entry table read
 *------------------------------------------
 */
static int grfio_entryread(char *gfname, int gentry) {
	FILE *fp;
	int grf_size,list_size;
	unsigned char grf_header[0x2e];
	int lop,entry,entrys,ofs,grf_version;
	unsigned char *fname;
	unsigned char *grf_filelist;

	if((fp = fopen(gfname,"rb"))==NULL) {
		printf("%s not found\n",gfname);
		return 1;	// 1:not found error
	}

	fseek(fp,0,2);	// SEEK_END
	grf_size = ftell(fp);
	fseek(fp,0,0);	// SEEK_SET
	fread(grf_header,1,0x2e,fp);
	if(strcmp(grf_header,"Master of Magic") || fseek(fp,getlong(grf_header+0x1e),1)) {	// SEEK_CUR
		fclose(fp);
		printf("%s read error\n",gfname);
		return 2;	// 2:file format error
	}

	grf_version = getlong(grf_header+0x2a) >> 8;

	if (grf_version==0x01) {	//****** Grf version 01xx ******
		list_size = grf_size-ftell(fp);
		grf_filelist = calloc(list_size, 1);
		if(grf_filelist==NULL) {
			fclose(fp);
			printf("out of memory : grf_filelist\n");
			return 3;	// 3:memory alloc error
		}
		fread(grf_filelist,1,list_size,fp);
		fclose(fp);

		entrys = getlong(grf_header+0x26) - getlong(grf_header+0x22) - 7;

		// Get an entry
		for(entry=0,ofs=0;entry<entrys;entry++) {
			int ofs2,srclen,srccount,type;
			char *period_ptr;
			FILELIST aentry;

			ofs2 = ofs+getlong(grf_filelist+ofs)+4;
			type = grf_filelist[ofs2+12];
			if (type!=0) {	// Directory Index ... skip
				fname = decode_filename(grf_filelist+ofs+6,grf_filelist[ofs]-6);
				if (strlen(fname) > sizeof(aentry.fn) - 1) {
					printf("file name too long : %s.\n", fname);
					FREE(grf_filelist);
					exit(1);
				}
				srclen=0;
				if((period_ptr=rindex(fname,'.'))!=NULL) {
					for(lop=0;lop<4;lop++) {
						if(strcmpi(period_ptr,".gnd\0.gat\0.act\0.str"+lop*5)==0)
							break;
					}
					srclen=getlong(grf_filelist+ofs2)-getlong(grf_filelist+ofs2+8)-715;
					if(lop==4) {
						for(lop=10,srccount=1;srclen>=lop;lop=lop*10,srccount++);
					} else {
						srccount=0;
					}
				} else {
					srccount=0;
				}

				aentry.srclen         = srclen;
				aentry.srclen_aligned = getlong(grf_filelist+ofs2+4)-37579;
				aentry.declen         = getlong(grf_filelist+ofs2+8);
				aentry.srcpos         = getlong(grf_filelist+ofs2+13)+0x2e;
				aentry.cycle          = srccount;
				aentry.type           = type;
				strncpy(aentry.fn,fname,sizeof(aentry.fn)-1);
#ifdef	GRFIO_LOCAL
				aentry.gentry         = -(gentry+1);	// As Flag for making it a negative number carrying out the first time LocalFileCheck
#else
				aentry.gentry         = gentry+1;		// With no first time LocalFileCheck
#endif
				filelist_modify(&aentry);
			}
			ofs = ofs2 + 17;
		}
		FREE(grf_filelist);

	} else if (grf_version==0x02) {	//****** Grf version 02xx ******
		unsigned char eheader[8];
		unsigned char *rBuf;
		uLongf rSize,eSize;

		fread(eheader,1,8,fp);
		rSize = getlong(eheader);	// Read Size
		eSize = getlong(eheader+4);	// Extend Size

		if (rSize > grf_size-ftell(fp)) {
			fclose(fp);
			printf("Illegal data format : grf compress entry size\n");
			return 4;
		}

		rBuf = calloc( rSize , 1);	// Get a Read Size
		if (rBuf==NULL) {
			fclose(fp);
			printf("out of memory : grf compress entry table buffer\n");
			return 3;
		}
		grf_filelist = calloc( eSize , 1);	// Get a Extend Size
		if (grf_filelist==NULL) {
			FREE(rBuf);
			fclose(fp);
			printf("out of memory : grf extract entry table buffer.\n");
			return 3;
		}
		fread(rBuf,1,rSize,fp);
		fclose(fp);
		decode_zip(grf_filelist,&eSize,rBuf,rSize);	// Decode function
		list_size = eSize;
		FREE(rBuf);

		entrys = getlong(grf_header + 0x26) - 7;

		// Get an entry
		for(entry=0,ofs=0;entry<entrys;entry++) {
			int ofs2,srclen,srccount,type;
			FILELIST aentry;

			fname = grf_filelist + ofs;
			if (strlen(fname) > sizeof(aentry.fn) - 1) {
				printf("grf : file name too long : %s.\n", fname);
				FREE(grf_filelist);
				exit(1);
			}
			ofs2 = ofs+strlen(grf_filelist+ofs)+1;
			type = grf_filelist[ofs2+12];
			if(type==1 || type==3 || type==5) {
				srclen=getlong(grf_filelist+ofs2);
				if (grf_filelist[ofs2+12]==3) {
					for(lop=10,srccount=1;srclen>=lop;lop=lop*10,srccount++);
				} else if (grf_filelist[ofs2+12]==5) {
					srccount = 0;
				} else {	// if (grf_filelist[ofs2+12]==1) {
					srccount = -1;
				}

				aentry.srclen         = srclen;
				aentry.srclen_aligned = getlong(grf_filelist+ofs2+4);
				aentry.declen         = getlong(grf_filelist+ofs2+8);
				aentry.srcpos         = getlong(grf_filelist+ofs2+13)+0x2e;
				aentry.cycle          = srccount;
				aentry.type           = type;
				strncpy(aentry.fn,fname,sizeof(aentry.fn)-1);
#ifdef	GRFIO_LOCAL
				aentry.gentry         = -(gentry+1);	// As Flag for making it a negative number carrying out the first time LocalFileCheck
#else
				aentry.gentry         = gentry+1;		// With no first time LocalFileCheck
#endif
				filelist_modify(&aentry);
			}
			ofs = ofs2 + 17;
		}
		FREE(grf_filelist);

	} else { //****** Grf Other version ******
		fclose(fp);
		printf("not support grf versions : %04x.\n", getlong(grf_header + 0x2a));
		return 4;
	}

	filelist_adjust(); // Unnecessary area release of filelist

	return 0;	// 0:no error
}

/*==========================================
 * Grfio : Resource file check
 *------------------------------------------
 */
static void grfio_resourcecheck() {
	int size;
	unsigned char *buf,*ptr;
	char w1[256],w2[256],src[256],dst[256];
	FILELIST *entry;

	buf=grfio_reads("data\\resnametable.txt",&size);
	buf[size] = 0;

	for(ptr=buf;ptr-buf<size;) {
		memset(w2, 0, sizeof(w2));
		if(sscanf(ptr,"%[^#]#%[^#]#",w1,w2)==2) {
			if(strstr(w2,"bmp")) {
				sprintf(src,"data\\texture\\%s",w1);
				sprintf(dst,"data\\texture\\%s",w2);
			} else {
				sprintf(src,"data\\%s",w1);
				sprintf(dst,"data\\%s",w2);
			}
			entry = filelist_find(dst);
			if (entry!=NULL) {
				FILELIST fentry;
				memcpy(&fentry, entry, sizeof(FILELIST));
				strncpy(fentry.fn ,src, sizeof(fentry.fn)-1);
				filelist_modify(&fentry);
			} else {
				//printf("file not found in data.grf : %s < %s\n",dst,src);
			}
		}
		ptr = strchr(ptr,'\n');	// Next line
		if (!ptr) break;
		ptr++;
	}
	FREE(buf);
	filelist_adjust();	// Unnecessary area release of filelist
}

/*==========================================
 * Grfio : Resource add
 *------------------------------------------
 */
#define	GENTRY_ADDS	16	// The number increment of gentry_table entries

int grfio_add(char *fname) {
	int len,result;
	char *buf;

	if (gentry_entrys>=GENTRY_LIMIT) {
		printf("gentrys limit : grfio_add\n");
		exit(1);
	}

	printf("%s file reading...\n",fname);

	if (gentry_entrys>=gentry_maxentry) {
		char **new_gentry = (char**)realloc((void*)gentry_table,(gentry_maxentry+GENTRY_ADDS)*sizeof(char*));
		if (new_gentry!=NULL) {
			int lop;
			gentry_table = new_gentry;
			gentry_maxentry += GENTRY_ADDS;
			for(lop=gentry_entrys;lop<gentry_maxentry;lop++)
				gentry_table[lop] = NULL;
		} else {
			printf("out of memory : grfio_add\n");
			exit(1);
		}
	}
	len = strlen(fname);
	buf = calloc(len + 1, 1);
	if (buf == NULL) {
		printf("out of memory : gentry\n");
		exit(1);
	}
	strcpy(buf, fname);
	gentry_table[gentry_entrys++] = buf;

	result = grfio_entryread(fname, gentry_entrys - 1);

	if (result == 0) {
		// Resource check
		grfio_resourcecheck();
	}

	return result;
}

/*==========================================
 * Grfio : Finalize
 *------------------------------------------
 */
void grfio_final(void) {
	int lop;

	FREE(filelist);
	filelist_entrys = filelist_maxentry = 0;

	if (gentry_table != NULL) {
		for(lop=0;lop<gentry_entrys;lop++) {
			if (gentry_table[lop] != NULL) {
				FREE(gentry_table[lop]);
			}
		}
		FREE(gentry_table);
	}
	gentry_entrys = gentry_maxentry = 0;
}

/*==========================================
 * Grfio : Initialize
 *------------------------------------------
 */
void grfio_init(char *fname) {
	FILE *data_conf;
	char line[1024], w1[1024], w2[1024];
	int result = 0, result2 = 0, result3 = 0, result4 = 0;

	data_conf = fopen(fname, "r");

	// It will read, if there is grf-files.txt.
	if (data_conf) {
		while(fgets(line, sizeof(line) - 1, data_conf)) {
			memset(w2, 0, sizeof(w2));
			if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) == 2) {
				if (strcmp(w1, "data") == 0)
					strcpy(data_file, w2);
				else if (strcmp(w1, "sdata") == 0)
					strcpy(sdata_file, w2);
				else if (strcmp(w1, "adata") == 0)
					strcpy(adata_file, w2);
				else if (strcmp(w1, "data_dir") == 0)
					strcpy(data_dir, w2);
			}
		}

		fclose(data_conf);
		printf("read %s done.\n",fname);
	} // end of reading grf-files.txt

	hashinit(); // hash table initialization

	filelist = NULL;
	filelist_entrys = filelist_maxentry = 0;
	gentry_table = NULL;
	gentry_entrys = gentry_maxentry = 0;
	atexit(grfio_final); // End processing definition

	// Entry table reading

	if (strcmp(data_file, "") != 0)			// If data directive exists in grf-files.txt (i.e. data_file is not equal to "")
		result = grfio_add(data_file);		// Primary data file

	if (strcmp(sdata_file, "") != 0)		// If sdata directive exists in grf-files.txt (i.e. sdata_file is not equal to "")
		result2 = grfio_add(sdata_file);	// Sakray data file

	if (strcmp(adata_file, "") != 0)		// If data directive exists in grf-files.txt (i.e. adata_file is not equal to "")
		result3 = grfio_add(adata_file);	// Alpha version data file

	if (strcmp(data_dir, "") == 0)			// If data_dir doesn't exist
		result4 = 1;						// Data directory

	if (result != 0 && result2 != 0 && result3 != 0 && result4 != 0) {
		printf("not grf file readed exit!!\n");
		exit(1); // It ends, if a resource cannot read one.
	}
}

