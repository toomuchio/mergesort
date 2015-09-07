#include "libutil.h"
#include <math.h>

int main (int argc, char **argv) {
	/*Declarations*/
	clock_t t = clock();
	FILE *inFile, *outFile;
	int i = 0, x = 0, a = 0, inPageSize = 0, inBufferSize = 0, inElements = 0, sizeSortBuffer = 0, numSortBuffers = 0;
	/*Structures*/
	int fileType = 0;
	list_t *pqueue;
	node_t *currentNode = NULL;
	data_t *tmpElement = NULL;
	data_t *outputBuffer = NULL;
	data_t **sortBuffer = NULL;
	data_t *joinChars = NULL;
	data_t *joinGuilds = NULL;
	int *bufferOffsets = NULL;
	int *bufferLimits = NULL;
	int *bufferStatus = NULL;
	/*Temporary*/
	char *outFileName = NULL;
	char tmpCName[40];
	int tmpTeam, tmpLevel, tmpCId, tmpGuildID;
	int tmpNumSortBuffers = 0, tmpSizeSortBuffer = 0, tmpFiles = 0, tmpCFile = 0, currentBuffer = 0, outputCount = 0, tmpWorking = 0;
	int joinLimit = 0;
	int joinExpected = 0;
	int joinFound = 0;

	/*Command Line Parameters*/
	if(argv[0] == NULL || argv[1] == NULL || argv[2] == NULL || argv[3] == NULL || argv[4] == NULL) {
		printf("Usage: %s <pagesize> <buffers> <path>/characters <path>/guilds\n", argv[0]);
		return EXIT_FAILURE;
	} else if (strcmp(argv[1], "-o") == 0) {
		if (argv[5] == NULL) {
			printf("Usage: %s <pagesize> <buffers> <path>/characters <path>/guilds\n", argv[0]);
			return EXIT_FAILURE;
		}
		tmpCFile = 4;
	} else {
		tmpCFile = 3;
	}

	/*Process both files*/
	while(tmpCFile != 5) {
		/*Reset the variables for the current file*/
		if (strcmp(argv[1], "-o") == 0) {
			inPageSize = atoi(argv[2]);
			inBufferSize = atoi(argv[3]);
		} else {
			inPageSize = atoi(argv[1]);
			inBufferSize = atoi(argv[2]);
		}
		sizeSortBuffer = inBufferSize * inPageSize;

		tmpNumSortBuffers = 0, tmpSizeSortBuffer = 0, tmpFiles = 0, currentBuffer = 0, outputCount = 0, tmpWorking = 0;
		i = 0, x = 0, a = 0, inElements = 0, numSortBuffers = 0;
		pqueue = safe_malloc(sizeof(list_t));

		/*Test the file hooks*/
		inFile = fopen(argv[tmpCFile], "r");
		if (inFile == NULL) {
			printf("Error - File %s could not be opened for reading.\n", argv[tmpCFile]);
			return EXIT_FAILURE;
		}

		outFile = fopen("_1.bin", "wb");
		if (outFile == NULL) {
			printf("Error - Temporary file could not be opened for writing.\n");
			return EXIT_FAILURE;
		}

		/*Allocate the maximum buffer per sort*/
		sortBuffer = safe_malloc(sizeof(data_t*) * sizeSortBuffer);

		/*Determine the file type*/
		if (fscanf(inFile, "%[^,],%d,%d,%d,%d\n", tmpCName, &tmpTeam, &tmpLevel, &tmpCId, &tmpGuildID) == 5) {
			fileType = 1;
		} else if (!fseek(inFile, 0, SEEK_SET) && fscanf(inFile, "%d,%[^\n]\n", &tmpGuildID, tmpCName) == 2) {
			fileType = 0;
		} else {
			printf("Error - Invalid input file.\n");
			return EXIT_FAILURE;
		}

		/*Loop the file sorting it in chunks and writing out once the buffer size is reached.*/
		fseek(inFile, 0, SEEK_SET);
		while ((fileType == 1 && fscanf(inFile, "%[^,],%d,%d,%d,%d\n", tmpCName, &tmpTeam, &tmpLevel, &tmpCId, &tmpGuildID) == 5) || (fileType == 0 && fscanf(inFile, "%d,%[^\n]\n", &tmpGuildID, tmpCName) == 2)) {
			inElements++;

			/*Create the Element*/
			tmpElement = safe_malloc(sizeof(data_t));
			if (fileType == 1) {
				strcpy(tmpElement->type.cdata.CName, tmpCName);
				tmpElement->type.cdata.Team = tmpTeam;
				tmpElement->type.cdata.Level = tmpLevel;
				tmpElement->type.cdata.CId = tmpCId;
				tmpElement->GuildID = tmpGuildID;
			} else if (fileType == 0) {
				tmpElement->GuildID = tmpGuildID;
				strcpy(tmpElement->type.GName, tmpCName);
			}
			sortBuffer[i++] = tmpElement;

			/*If the buffer size has been reached, sort the elements and write them to a binary file.*/
			if (i == sizeSortBuffer) {
				quicksort(sortBuffer, 0, i-1);

				for(x=0; x < i; x++) {
					fwrite(sortBuffer[x], sizeof(data_t), 1, outFile);
					free(sortBuffer[x]);
					sortBuffer[x] = NULL;
				}

				i = 0;
			}
		}

		/*Ensure the buffer is empty, there may not be an even distribution*/
		if (i != 0) {
			quicksort(sortBuffer, 0, i-1);

			for(x=0; x < i; x++) {
				fwrite(sortBuffer[x], sizeof(data_t), 1, outFile);
				free(sortBuffer[x]);
				sortBuffer[x] = NULL;
			}

			i = 0;
		}

		/*Close file hooks*/
		free(sortBuffer);
		fclose(inFile);
		fclose(outFile);

		/*Begin Merge*/
		numSortBuffers = inElements / sizeSortBuffer;
		inBufferSize--; /*One becomes the output buffer so we cannot use it...*/

		printf("Buffers: %d\nPages Per Buffer: %d\nTotal Elements: %d\nTotal Elements Per Buffer: %d\nNumber of Sorted Buffers: %d\n", inBufferSize, inPageSize, inElements, sizeSortBuffer, numSortBuffers);

		while (tmpNumSortBuffers != 1) {
			/*Reset*/
			pqueue->head = NULL;
			currentNode = NULL;
			currentBuffer = 0;
			tmpNumSortBuffers = 0;
			tmpSizeSortBuffer = 0;

			/*Open the relevent working file*/
			if (tmpFiles == 0) {
				inFile = fopen("_1.bin", "rb");
				outFile = fopen("_2.bin", "wb");
			} else {
				inFile = fopen("_2.bin", "rb");
				outFile = fopen("_1.bin", "wb");
			}
			tmpFiles ^= 1;

			/*Bulid the buffer offset arrays I could perform operations during the loops to work out the offsets, but working them out ONCE and storing them results in less mathematical overhead*/
			bufferOffsets = safe_malloc(sizeof(int) * (numSortBuffers+1));
			bufferLimits = safe_malloc(sizeof(int) * (numSortBuffers+1));
			bufferStatus = safe_malloc(sizeof(int) * (numSortBuffers));
			for(a = 0; a < numSortBuffers; a++) {
				bufferOffsets[a] = (sizeSortBuffer * a) * sizeof(data_t);
				bufferLimits[a] = (sizeSortBuffer * a) * sizeof(data_t);
				bufferStatus[a] = 0;
			}
			fseek(inFile, 0, SEEK_END);
			bufferOffsets[a] = ftell(inFile);
			bufferLimits[a] = ftell(inFile);

			/*Loop untill we've gone through all the sorted buffers and merged them, including any merged buffers we create in the process*/
			while(currentBuffer != numSortBuffers) {
				/*Fill the pqueue with the initial vaules*/
				a = 0;
				while (a != inBufferSize) {
					if (currentBuffer == numSortBuffers) { /*Sanity Check*/
						break;
					}

					/*Work out how many elements are in the current buffer, if there's more than we can fit in memory cap it*/
					tmpWorking = (bufferLimits[currentBuffer+1] - bufferOffsets[currentBuffer]) / sizeof(data_t);
					if (tmpWorking > inPageSize) {
						tmpWorking = inPageSize;
					}

					/*If the buffer wasn't empty, read in the maximum ammount and insert it into the queue*/
					if (tmpWorking != 0) {
						fseek(inFile, bufferOffsets[currentBuffer], SEEK_SET);

						/*Used to reduce IO operations, but since one big block of memory is created we now need to split it into chucks.*/
						tmpElement = safe_malloc(tmpWorking * sizeof(data_t));
						fread(tmpElement, sizeof(data_t), tmpWorking, inFile);

						for(x=0; x < tmpWorking; x++) {
							data_t *point = safe_malloc(sizeof(data_t));
							memcpy(point, &tmpElement[x], sizeof(data_t));

							pqueue_insert(pqueue, point, currentBuffer);
							bufferOffsets[currentBuffer] += sizeof(data_t);
							bufferStatus[currentBuffer]++;
						}
						free(tmpElement);
					}

					currentBuffer++;
					a++;
				}

				/*While there's elements in the queue*/
				outputBuffer = safe_malloc(sizeof(data_t) * inPageSize);
				while((currentNode = pqueue_dequeue(pqueue)) != NULL) {
					/*Keep track of the number of buffers*/
					if (tmpNumSortBuffers == 0) {
						tmpSizeSortBuffer++;
					}

					/*If the output buffer is full, push it out before we attempt to fill it with a new record. Done with a single IO operation.*/
					if(outputCount == inPageSize) {
						fwrite(outputBuffer, sizeof(data_t), outputCount, outFile);

						/*Reset the output buffer, */
						free(outputBuffer);
						outputBuffer = safe_malloc(sizeof(data_t) * inPageSize);
						outputCount = 0;
					}

					/*Move the element to the output buffer*/
					memmove(&outputBuffer[outputCount++], currentNode->payload, sizeof(data_t));
					free(currentNode->payload);
					bufferStatus[currentNode->offset]--;

					/*Buffer is empty? Fill it up again we don't want to fill bit by bit as it may cause issues with the queue and this way IO operations are reduced*/
					if (bufferStatus[currentNode->offset] == 0) {
						tmpWorking = (bufferLimits[currentNode->offset+1] - bufferOffsets[currentNode->offset]) / sizeof(data_t);
						if (tmpWorking > inPageSize) {
							tmpWorking = inPageSize;
						}

						if (tmpWorking != 0) {
							fseek(inFile, bufferOffsets[currentNode->offset], SEEK_SET);

							tmpElement = safe_malloc(tmpWorking * sizeof(data_t));
							fread(tmpElement, sizeof(data_t), tmpWorking, inFile);

							for(x=0; x < tmpWorking; x++) {
								data_t *point = safe_malloc(sizeof(data_t));
								memcpy(point, &tmpElement[x], sizeof(data_t));

								pqueue_insert(pqueue, point, currentNode->offset);
								bufferOffsets[currentNode->offset] += sizeof(data_t);
								bufferStatus[currentNode->offset]++;
							}
							free(tmpElement);
						}
					}

					/*Free the pqueue node*/
					free(currentNode);
				}

				/*Empty the output buffer*/
				if(outputCount != 0) {
					fwrite(outputBuffer, sizeof(data_t), outputCount, outFile);
					outputCount = 0;
				}
				free(outputBuffer);

				tmpNumSortBuffers++;
			}

			printf("Merged > %d buffers with at most %d records per buffer\n", numSortBuffers, sizeSortBuffer);
			numSortBuffers = tmpNumSortBuffers;
			sizeSortBuffer = tmpSizeSortBuffer;

			/*Clean Up*/
			free(bufferOffsets);
			free(bufferLimits);
			free(bufferStatus);
			fclose(outFile);
			fclose(inFile);
		}
		free(pqueue);

		/*Create the output file name*/
		outFileName = safe_malloc(strlen(argv[tmpCFile])+1 + 8);
		strcat(outFileName, argv[tmpCFile]);
		strcat(outFileName, ".bin");

		/*Rename the relevent file for joining later*/
		if (tmpFiles == 0) {
			rename("_1.bin", outFileName);
		} else {
			rename("_2.bin", outFileName);
		}
		free(outFileName);

		/*Remove working files*/
		remove("_1.bin");
		remove("_2.bin");

		tmpCFile++;
	}

	/*Work out the ammount of chars and guilds we can store in memory at a time, +1 is due to now being able to use the output buffer.*/
	joinLimit = (inBufferSize+1) * inPageSize / 2;

	/*Open the file hooks, work out how many records we SHOULD find.*/
	inFile = fopen("guilds.bin", "rb");
	joinGuilds = safe_malloc(joinLimit * sizeof(data_t));
	fread(joinGuilds, sizeof(data_t), joinLimit, inFile);

	outFile = fopen("characters.bin", "rb");
	fseek(outFile, 0, SEEK_END);
	joinExpected = ftell(outFile) / sizeof(data_t);
	fseek(outFile, 0, SEEK_SET);
	joinChars = safe_malloc(joinLimit * sizeof(data_t));
	fread(joinChars, sizeof(data_t), joinLimit, outFile);

	x = 0, i = 0;
	/*Join Chars on Guilds*/
	if (fileType == 0) {
		while (joinFound != joinExpected) {
			/*Align the two files*/
			while(joinGuilds[i].GuildID < joinChars[x].GuildID) {
				i++;
				if (i == joinLimit) {
					fread(joinGuilds, sizeof(data_t), joinLimit, inFile);
					i = 0;
				}
			}

			/*Align the two files*/
			while(joinGuilds[i].GuildID > joinChars[x].GuildID) {
				x++;
				if (x == joinLimit) {
					fread(joinChars, sizeof(data_t), joinLimit, outFile);
					x = 0;
				}
			}

			/*If the two are equal write it to the output and increment the count*/
			if(joinGuilds[i].GuildID == joinChars[x].GuildID) {
				if (strcmp(argv[1], "-o") == 0) {
					printf("%s,%d,%d,%d,%s\n", joinChars[x].type.cdata.CName, joinChars[x].type.cdata.Team, joinChars[x].type.cdata.Level, joinChars[x].type.cdata.CId, joinGuilds[i].type.GName);
				}
				joinFound++;
				x++;
				if (x == joinLimit) {
					fread(joinChars, sizeof(data_t), joinLimit, outFile);
					x = 0;
				}
			}
		}
	/*Join Guilds on Chars*/
	} else {
		/*Align the two files*/
		while (joinFound != joinExpected) {
			while(joinGuilds[i].GuildID < joinChars[x].GuildID) {
				i++;
				if (i == joinLimit) {
					fread(joinGuilds, sizeof(data_t), joinLimit, inFile);
					i = 0;
				}
			}

			/*Align the two files*/
			while(joinGuilds[i].GuildID > joinChars[x].GuildID) {
				x++;
				if (x == joinLimit) {
					fread(joinChars, sizeof(data_t), joinLimit, outFile);
					x = 0;
				}
			}

			/*If the two are equal write it to the output and increment the count*/
			while(joinGuilds[i].GuildID == joinChars[x].GuildID) {
				if (strcmp(argv[1], "-o") == 0) {
					printf("%s,%d,%d,%d,%s\n", joinChars[x].type.cdata.CName, joinChars[x].type.cdata.Team, joinChars[x].type.cdata.Level, joinChars[x].type.cdata.CId, joinGuilds[i].type.GName);
				}
				joinFound++;
				x++;
				if (x == joinLimit) {
					fread(joinChars, sizeof(data_t), joinLimit, outFile);
					x = 0;
				}
			}
			if (joinGuilds[i].GuildID == joinChars[x-1].GuildID) {
				i++;
			}
		}
	}

	free(joinGuilds);
	free(joinChars);
	fclose(inFile);
	fclose(outFile);

	printf("Number of tuples: %d\n", joinFound);
	printf("Time: %f\n", ((double)(clock() - t))/CLOCKS_PER_SEC);
	return EXIT_SUCCESS;
}

