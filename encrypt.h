#ifndef VARINT
#include "variableint.h"
#endif

typedef uint16_t BLOCKDATA;

struct lenstr
{
    char *data;
    BLOCKDATA len;
};
typedef struct lenstr lengthString;

lengthString encode(char *data, int *inputFormat, int *outputFormat, int datalen, int inputFormatLen, int outputFormatLen, bool verbose);

/*
// Basic plan: Convert from base (inputFormatSize) to base (outputFormatSize), they shouldn't share any traits
        // Convert the data from the open file into the VariableInt
        
        // Basic plan: Convert from base (inputFormatSize) to base (outputFormatSize), they shouldn't share any traits
        // Convert the data from the open file into the VariableInt
        {
            VariableInt multer = variableInt();
            setVarInt(&multer, inputFormatSize);
            int dataRead;
            while ((dataRead = fgetc(targetFile)) != EOF)
            {
                unsigned char reformat = inputFormat[dataRead];
                multVarInt(dataVarInteger, &multer, dataVarInteger);
                dataVarInteger->data[0] += reformat; // Insert the character here. What could possibly go wrong?
            }
            destroyVarInt(multer);
        }

        fclose(targetFile);
        if (verbose)
        {
            printf("Numerical representation obtained\n");
        }
        // Read the VariableInt back out, in a different base encrypted in the output symbol format
        {

            int blocklen = bitlenVarInt(dataVarInteger) / ((outputFormatSize == 2) ? 2 : 1);
            int pos = blocklen - 1;
            if (verbose)
            {
                printf("Allocating memory for output\n");
            }
            char *newStr = malloc(blocklen); // This will be the maximum size needed to store the base conversion
            if (verbose)
            {
                printf("Memory successfully allocated\n");
            }
            VariableInt outputLen = variableInt();
            outputLen.data[0] = outputFormatSize;
            while (dataVarInteger->data[0] != 0 || dataVarInteger->len > 1)
            {
                newStr[pos] = outputFormat[(unsigned char)divVarInt(dataVarInteger, &outputLen, dataVarInteger)];
                pos--;
            }
            destroyVarInt(outputLen);
    
            FILE *outputFile = fopen(outputFileNM, "wb");
            if (!outputFile)
            {
                printf("The output file \"%s\" was not able to be created!\n", outputFileNM);
                exit(EXIT_FAILURE);
            }
            for (pos++; pos < blocklen; pos++)
            {
                fputc(newStr[pos], outputFile);
            }
            fclose(outputFile);
        }


        // Convert the data from the open file into the VariableInt
        int inputUnformat[256];
        int outputUnformat[256];
        unformat(inputFormat, inputUnformat, inputFormatSize);
        unformat(outputFormat, outputUnformat, outputFormatSize);
        {
            // Get the contents of the file
            VariableInt multer = variableInt();
            setVarInt(&multer, outputFormatSize);
            int dataRead = 0;
            while ((dataRead = getc(targetFile)) != EOF)
            {
                int reformat = outputUnformat[(unsigned) dataRead];
                if (reformat == -1)
                {
                    fprintf(stderr, "The output format was insufficient to decrypt this file.\n");
                    exit(EXIT_FAILURE);
                }
                multVarInt(dataVarInteger, &multer, dataVarInteger);
                dataVarInteger->data[0] += (unsigned char) reformat;
            }
            destroyVarInt(multer);
        }
        if (verbose)
        {
            printf("Numerical representation obtained\n");
        }
        fclose(targetFile);
        {
            int blocklen = bitlenVarInt(dataVarInteger) / ((outputFormatSize == 2) ? 2 : 1);
            int pos = blocklen - 1;
            if (verbose)
            {
                printf("Allocating memory for output\n");
            }
            char *newStr = malloc(blocklen); // This will be the maximum size needed to store the base conversion
            if (verbose)
            {
                printf("Memory successfully allocated\n");
            }
            VariableInt inputLen = variableInt();
            inputLen.data[0] = inputFormatSize;
            while (dataVarInteger->data[0] != 0 || dataVarInteger->len > 1)
            {
                newStr[pos] = inputUnformat[(unsigned char)divVarInt(dataVarInteger, &inputLen, dataVarInteger)];
                pos--;
            }
            destroyVarInt(inputLen);

            FILE *outputFile = fopen(outputFileNM, "wb");
            if (!outputFile)
            {
                printf("The output file \"%s\" was not able to be created!\n", outputFileNM);
                exit(EXIT_FAILURE);
            }
            
            for (pos++; pos < blocklen; pos++)
            {
                fputc(newStr[pos], outputFile);
            }
            fclose(outputFile);
        }
*/