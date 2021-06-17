// We begin by including stdlib, string, stdio, and stdbool
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

#include "variableint.h"
#include "encrypt.h"

enum CommandFlag
{
    NONE,
    O,
    OF,
    IF,
};

#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

// Boolean flag initialization
bool verbose = false;
bool encrypt = true;

// Computes the number of distinct characters
int nrDisChar(FILE *file);
// Checks for duplicates in the format keys
bool testDuplicates(int *data, int len);
// "Reverses" a key
void unformat(int *format, int new[256], int formatlen);

#define BLOCK_LENGTH 512

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        printf("\a\033[33mNo arguments included! Use \"bxe TARGET (optional: -e)\" to encrypt a file, or use bxe --help. \n\033[0m");
        return 1;
    }

    if (strcmp(argv[1], "--help") == 0)
    {
        printf("Usage: bxe [-e|-d] [-o OUTPUT] [-if INPUTKEY] [-of OUTPUTKEY] [-v] [-f] TARGET\n");
        printf("TARGET: The name of the file to be encrypted/decrypted.\n");
        printf("-e: Generates an encrypted file\n");
        printf("-d: Generates a decrypted file\n");
        printf("-o OUTPUT: Provides the name of the output file. If not included, the file itself will be replaced with an encrypted form of its contents.\n");
        printf("-v: Verbose mode active\n");
        printf("-f: Does not ask for any further input\n");
        printf("All flags are case-sensitive.\n");
        printf("To show this again, type bxe --help.\n");
        return 0;
    }

    char* targetFileNM = 0; // These will be set later
    char* outputFileNM = 0;
    char* inputFormatNM = 0;
    char* outputFormatNM = 0;

    bool force = false;
    enum CommandFlag flagType = NONE;
    // Read in command-line arguments
    for (int argi = 1; argi < argc; argi++)
    {
        char* argCur = argv[argi];

        // Cases where a flag is linked
        if (flagType != NONE)
        {
            switch (flagType)
            {
                case O:
                    outputFileNM = argCur;
                    break;
                case IF:
                    inputFormatNM = argCur;
                    break;
                case OF:
                    outputFormatNM = argCur;
                    break;
            }
            flagType = NONE;
            continue;
        }
        // Testing for flags
        if (strcmp("-v", argCur) == 0)
        {
            verbose = true;
            continue;
        }
        if (strcmp("-o", argCur) == 0)
        {
            flagType = O;
            continue;
        }
        if (strcmp("-if", argCur) == 0)
        {
            flagType = IF;
            continue;
        }
        if (strcmp("-of", argCur) == 0)
        {
            flagType = OF;
            continue;
        }
        if (strcmp("-e", argCur) == 0)
        {
            encrypt = true;
            continue;
        }
        if (strcmp("-d", argCur) == 0)
        {
            encrypt = false;
            continue;
        }
        if (strcmp("-f", argCur) == 0)
        {
            force = true;
            continue;
        }
        // Everything else should have been covered by now, let's get that filename (if we haven't already allocated it)!
        if (targetFileNM == 0)
        {
            targetFileNM = malloc(strlen(argCur) + 1);
            strcpy(targetFileNM, argCur);
        }
    }

    if (!targetFileNM)
    {
        fprintf(stderr, "\033[31mError: No input file was specified, use bxe --help for further information. \033[0m");
        return EXIT_FAILURE;
    }

    FILE *targetFile = fopen(targetFileNM, "rb");
    if (!targetFile)
    {
        fprintf(stderr, "\033[31mError: No such file \"%s\" exists!\n\033[0m", targetFileNM);
        return EXIT_FAILURE;
    }
    int inputCharCount = nrDisChar(targetFile);

    if (!outputFileNM && !force)
    {
        printf("You have not supplied any output file with the flag -o: if you press (Y/y) to continue, you will overwrite the targeted file. Are you sure you want to do this? (N/n to cancel): ");
        char c = 0;
        while (tolower(c) != 'y' && tolower(c) != 'n')
        {
            c = getchar();
        }
        if (tolower(c) == 'n')
        {
            printf("Operation canceled.\n");
            return EXIT_SUCCESS;
        }
        outputFileNM = targetFileNM;
    }
    else
    {
        free(targetFileNM);
    }

    // Next, we need info on our input and output sets (if they exist, otherwise, we use default settings)

    int outputFormat[256];
    memset(outputFormat, -1, sizeof(int));
    int outputFormatSize;
    if (outputFormatNM)
    {
        outputFormatSize = 0;
        FILE *outputFormatFile = fopen(outputFormatNM, "rb");
        int i;
        while ((i = fgetc(outputFormatFile)) != EOF && outputFormatSize <= 256)
        {
            outputFormat[outputFormatSize] = (int)(unsigned char)i;
            outputFormatSize++;
        }
        fclose(outputFormatFile);
        if (testDuplicates(outputFormat, outputFormatSize))
        {
            fprintf(stderr, "\033[31mError: The supplied output format file contains duplicate entries!\n\033[0m");
            return EXIT_FAILURE;
        }
        if (outputFormatSize < 2)
        {
            fprintf(stderr, "\033[31mError: The supplied output file does not contain sufficient information for encrypting this file!\n\033[0m");
            return EXIT_FAILURE;
        }
    }
    else
    {
        outputFormatSize = 255;
        for (int i = 0; i < 255; i++)
        {
            outputFormat[i] = (unsigned char)(256 - i * 3);
        }
    }

    // Analyze the input format file
    int inputFormat[256];
    memset(inputFormat, -1, sizeof(int));
    int inputFormatSize;
    if (inputFormatNM)
    {
        inputFormatSize = 0;
        FILE *inputFormatFile = fopen(inputFormatNM, "rb");
        int i;
        while ((i = fgetc(inputFormatFile)) != EOF && inputFormatSize <= 256)
        {
            inputFormat[inputFormatSize] = (int)(unsigned char)i;
            inputFormatSize++;
        }
        fclose(inputFormatFile);
        if (testDuplicates(inputFormat, inputFormatSize))
        {
            fprintf(stderr, "\033[31mError: The supplied input format file contains duplicate entries!\n\033[0m");
            return EXIT_FAILURE;
        }

        if (inputCharCount > inputFormatSize || inputFormatSize < 2)
        {
            fprintf(stderr, "\033[31mError: The supplied input file does not contain sufficient information for encrypting this file!\n\033[0m");
            return EXIT_FAILURE;
        }
    }
    else
    {
        inputFormatSize = 256;
        for (int i = 0; i < 256; i++)
        {
            inputFormat[i] = (unsigned char)(i * 7 + 4);
        }
    }

    if (inputFormatSize == outputFormatSize)
    {
        printf("Note: The input format is equal in size to the output format, which will greatly decrease the security of this algorithm. Press Ctrl+C to exit.\n");
    }

    if (encrypt)
    {
        // Basic plan: Convert from base (inputFormatSize) to base (outputFormatSize), they shouldn't share any traits
        // Convert the data from the open file into the VariableInt
        int inputUnformat[256];
        unformat(inputFormat, inputUnformat, inputFormatSize);
        fseek(targetFile, 0, SEEK_END);
        int targetLen = ftell(targetFile);
        fseek(targetFile, 0, SEEK_SET);
        char *targetData = malloc(targetLen * sizeof(char));
        fread(targetData, sizeof(char), targetLen, targetFile);
        fclose(targetFile);
        FILE *outputFile = fopen(outputFileNM, "wb");
        if (!outputFile)
        {
            printf("\033[31mError: The output file \"%s\" was not able to be created!\n\033[0m", outputFileNM);
            return EXIT_FAILURE;
        }

        for (int i = 0; i < targetLen / BLOCK_LENGTH; i++)
        {
            lengthString encodedData = encode(targetData + i * BLOCK_LENGTH, inputUnformat, outputFormat, BLOCK_LENGTH, inputFormatSize, outputFormatSize, verbose);
            fwrite(&encodedData.len, 1, sizeof(BLOCKDATA), outputFile); // Write the encrypted length, then the original length
            BLOCKDATA siz = BLOCK_LENGTH;
            fwrite(&siz, 1, sizeof(BLOCKDATA), outputFile);
            fwrite(encodedData.data, encodedData.len, sizeof(char), outputFile);
            free(encodedData.data);
            if (verbose)
            {
                // I originally learned this technique from StackOverflow, but I have modified it in certain cases here to fit my needs.
                double percentage = (double)i / (targetLen / BLOCK_LENGTH);
                int val = (int)(percentage * 100);
                int lpad = (int)(percentage * PBWIDTH);
                int rpad = PBWIDTH - lpad;
                printf("\033[32m\r%3d%% [%.*s%*s]\033[0m", val, lpad, PBSTR, rpad, "");
                fflush(stdout);
            }
        }

        if (targetLen % BLOCK_LENGTH != 0)
        {
            lengthString encodedData = encode(targetData + (targetLen - (targetLen % BLOCK_LENGTH)), inputUnformat, outputFormat, targetLen % BLOCK_LENGTH, inputFormatSize, outputFormatSize, verbose);
            fwrite(&encodedData.len, 1, sizeof(BLOCKDATA), outputFile);
            BLOCKDATA siz = targetLen % BLOCK_LENGTH;
            fwrite(&siz, 1, sizeof(BLOCKDATA), outputFile);
            fwrite(encodedData.data, encodedData.len, sizeof(char), outputFile);
            free(encodedData.data);
            if (verbose)
            {
                printf("\033[32m\r%3d%% [%.*s%*s]\033[0m", 100, PBWIDTH, PBSTR, 0, "");
                fflush(stdout);
            }
        }
        free(targetData);
        fclose(outputFile);
    }
    else // Time to decrypt!
    {
        // Basic plan: Convert from base (inputFormatSize) to base (outputFormatSize), they shouldn't share any traits
        // Convert the data from the open file into the VariableInt
        int outputUnformat[256];
        unformat(outputFormat, outputUnformat, outputFormatSize);

        fseek(targetFile, 0, SEEK_END);
        int targetLen = ftell(targetFile);
        fseek(targetFile, 0, SEEK_SET);
        char *targetData = malloc(targetLen * sizeof(char));
        fread(targetData, sizeof(char), targetLen, targetFile);

        fclose(targetFile);

        FILE *outputFile = fopen(outputFileNM, "wb");
        if (!outputFile)
        {
            printf("\033[31mError: The output file \"%s\" was not able to be created!\n\033[0m", outputFileNM);
            return EXIT_FAILURE;
        }

        int offset = 0;
        while (offset < targetLen)
        {
            BLOCKDATA blockLen = *(BLOCKDATA *)(targetData + offset);
            offset += sizeof(BLOCKDATA);
            BLOCKDATA originalLen = *(BLOCKDATA *)(targetData + offset);
            offset += sizeof(BLOCKDATA);
            lengthString encodedData = encode(targetData + offset, outputUnformat, inputFormat, blockLen, outputFormatSize, inputFormatSize, verbose);
            if (encodedData.len < originalLen)
            {
                char *newData = malloc(originalLen * sizeof(char));
                for (int i = 0; i < originalLen; i++)
                {
                    newData[i] = i < (originalLen - encodedData.len) ? inputFormat[0] : encodedData.data[i - (originalLen - encodedData.len)];
                }
                free(encodedData.data);
                encodedData.data = newData;
            }
            fwrite(encodedData.data, originalLen, sizeof(char), outputFile);
            free(encodedData.data);
            offset += blockLen;
            if (verbose)
            {
                double percentage = (double)offset/targetLen;
                int val = (int)(percentage * 100);
                int lpad = (int)(percentage * PBWIDTH);
                int rpad = PBWIDTH - lpad;
                printf("\033[32m\r%3d%% [%.*s%*s]\033[0m", val, lpad, PBSTR, rpad, "");
                fflush(stdout);
            }
        }
        free(targetData);
        fclose(outputFile);
    }
    if (verbose)
    {
        printf("\033[32m\nOperation completed successfully!\n\033[0m");
    }
    //End the program by destroying our char*s and exitting
    if (targetFileNM == outputFileNM && targetFileNM)
    {
        free(targetFileNM);
    }
    return EXIT_SUCCESS;
}

// Determines the number of distinct characters in a file
int nrDisChar(FILE *file)
{
    bool *detected = calloc(256, sizeof(bool));
    unsigned char tester;
    int amount = 0;
    int i;
    while ((i = fgetc(file)) != EOF)
    {
        tester = (unsigned char)i;
        if (!detected[tester])
        {
            amount++;
            detected[(unsigned char)tester] = true;
        }
    }
    fseek(file, 0, SEEK_SET);
    free(detected);
    return amount;
}

int compare(const void* a, const void* b)
{
    int an = *(int*) a;
    int bn = *(int*) b;
    return an - bn;
}

bool testDuplicates(int *data, int len)
{
    int *data2 = malloc(len * sizeof(int));
    memcpy(data2, data, len * sizeof(int));
    qsort(data2, len, sizeof(int), compare); // I learned about this from StackOverflow, but I made my own sort function for this particular instance.
    for (int i = 0; i < len - 1; i++)
    {
        if (data2[i] == data2[i + 1])
        {
            return true;
        }
    }
    return false;
}

void unformat(int *format, int new[256], int formatlen)
{
    memset(new, -1, 256 * sizeof(int));
    for (int i = 0; i < formatlen; i++)
    {
        new[(unsigned char)format[i]] = i;
    }
}