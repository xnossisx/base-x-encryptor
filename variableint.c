#include "variableint.h"
#include <stdio.h>
#include <limits.h>

// Constructors

// Create a VariableInt of size 1
VariableInt variableInt()
{
    VariableInt ret;
    ret.len = 1;
    ret.data = calloc(1, sizeof(uint64_t));
    return ret;
}

// Create a VariableInt with a size of len allocated
VariableInt variableIntL(unsigned int len)
{
    VariableInt ret;
    ret.len = len;
    ret.data = calloc(len, sizeof(uint64_t));
    return ret;
}

// Create a copy of a VariableInt
VariableInt variableIntC(VariableInt *vi)
{
    VariableInt ret;
    ret.len = vi->len;
    ret.data = malloc(vi->len * sizeof(uint64_t));
    for (int i = 0; i < vi->len; i++)
    {
        ret.data[i] = vi->data[i];
    }
    return ret;
}

// Removes all allocated memory from a VariableInt
void destroyVarInt(VariableInt vi)
{
    free(vi.data);
    vi.data = NULL; // Mark that it is not assigned (debugging purposes)
}

// Determines the amount of blocks required to store all non-zero information
int getNeededLen(VariableInt *vi)
{
    int needed = vi->len;
    for (int i = vi->len - 1; i >= 1 && !vi->data[i]; i--)
    {
        needed--;
    }
    return needed;
}

// Remove any leading zeroes from a VariableInt
void compactVarInt(VariableInt *vi)
{
    int needed = getNeededLen(vi);
    if (needed == vi->len)
    {
        return;
    }
    VariableInt vnew = variableIntL(needed);
    for (int i = 0; i < needed; i++)
    {
        vnew.data[i] = vi->data[i];
    }
    destroyVarInt(*vi);
    *vi = vnew;
}

// Finds the non-zero leading block of vi
uint64_t leadVarInt(VariableInt *vi)
{
    return vi->data[getNeededLen(vi) - 1];
}

// Tests for equality between vi1 and vi2
bool eqVarInt(VariableInt *vi1, VariableInt *vi2)
{
    if (vi1 == vi2)
    {
        return true;
    }
    for (int i = 0; i < minVL(vi1, vi2); i++)
    {
        if (vi1->data[i] != vi2->data[i])
        {
            return false;
        }
    }
    return leadVarInt(vi1) == leadVarInt(vi2);
}

// Set the bottom element of a VariableInt
void setVarInt(VariableInt *vout, uint64_t set)
{
    vout->data[0] = set;
}

// Returns the larger VariableInt of the two passed in, and the second argument is selected in case of equality
VariableInt *maxV(VariableInt *vi1, VariableInt *vi2)
{
    int len1 = getNeededLen(vi1);
    int len2 = getNeededLen(vi2);
    return (len1 == len2) ? ((leadVarInt(vi1) > leadVarInt(vi2)) ? vi1 : vi2) : (len1 > len2 ? vi1 : vi2);
}

VariableInt *maxVP(VariableInt *vi1, VariableInt *vi2, int len1, int len2)
{
    if (len1 == len2)
    {
        len1--;
        while (vi1->data[len1] == vi2->data[len1] && len1 != 0)
        {
            len1--;
        }
        return (vi1->data[len1] > vi2->data[len1]) ? vi1 : vi2;
    }
    return (len1 > len2 ? vi1 : vi2);
}

// Returns the larger length of the two VariableInt passed in
int maxVL(VariableInt *vi1, VariableInt *vi2)
{
    int len1 = getNeededLen(vi1);
    int len2 = getNeededLen(vi2);
    return (len1 > len2) ? len1 : len2;
}

// Returns the smaller length of the two VariableInt passed in
int minVL(VariableInt *vi1, VariableInt *vi2)
{
    int len1 = getNeededLen(vi1);
    int len2 = getNeededLen(vi2);
    return (len1 < len2) ? len1 : len2;
}

// Computes the sum of two VariableInt, which is fed into vout
void addVarInt(VariableInt *vi1, VariableInt *vi2, VariableInt *vout)
{
    VariableInt viNew;
    int vi1len = getNeededLen(vi1);
    int vi2len = getNeededLen(vi2);
    if (vi1->data[vi1len - 1] > LLONG_MAX || vi2->data[vi2len - 1] > LLONG_MAX)
    {
        viNew = variableIntL(max(vi1len, vi2len) + 1);
    }
    else
    {
        viNew = variableIntL(max(vi1len, vi2len));
    }

    bool carry = false;
    int i = 0;
    for (; i < min(vi1len, vi2len); i++)
    {
        viNew.data[i] = vi1->data[i] + vi2->data[i] + carry;
        carry = viNew.data[i] < vi1->data[i]; // Fast way to detect if a carry bit should be set or not
    }

    if (carry)
    {
        viNew.data[i] = 1;
    }

    if (vi1len != vi2len)
    {
        VariableInt *viRem = maxVP(vi1, vi2, vi1len, vi2len);
        for (; i < viRem->len; i++)
        {
            carry = !(viRem->data[i] + 1); // Fast way to detect if a carry bit should be set or not
            viNew.data[i] += viRem->data[i] + carry;
        }
    }

    // Clear out any pointers (if vi1 or vi2 are identical to vout, then they will be destroyed here too)
    destroyVarInt(*vout);

    *vout = viNew;
}

void addVarIntIP(VariableInt *vi, uint64_t ins)
{
    bool carry = (vi->data[0] + ins < vi->data[0]);
    vi->data[0] += ins;
    if (carry)
    {
        int i = 0;
        do
        {
            i++;
            vi->data[i]++;
        }
        while (!vi->data[i] && i < vi->len);
        if (vi->len == i)
        {
            vi->data = realloc(vi->data, ++vi->len);
            vi->data[i] = 1;
        }
    }
}

// Left shift a VariableInt, output (vi << ls) is written to vout
void lshiftVarInt(VariableInt *vi, VariableInt *vout, unsigned int ls)
{
    VariableInt vnew = variableIntL(vi->len + 1 + (ls >> 6));
    uint64_t piece = 0;
    for (int i = 0; i < vi->len; i++)
    {
        vnew.data[i] = vi->data[i] << ls;
        vnew.data[i] += piece;
        piece = vi->data[i] >> (64 - ls);
    }
    vnew.data[vnew.len - 1] = piece;
    destroyVarInt(*vout);
    compactVarInt(&vnew);
    *vout = vnew;
}

// Adds two VariableInt together and feeds output into vout.
void multVarInt(VariableInt *vi1, VariableInt *vi2, VariableInt *vout)
{
    VariableInt grow = variableIntC(vi1);
    VariableInt vnew = variableIntL(vi1->len + vi2->len);
    for (int i = 0; i < getNeededLen(vi2); i++)
    {
        for (int j = 0; j < sizeof(uint64_t) * 8; j++)
        {
            if (vi2->data[i] & ((uint64_t)1 << j))
            {
                addVarInt(&vnew, &grow, &vnew);
            }
            lshiftVarInt(&grow, &grow, 1);
        }
    }
    destroyVarInt(grow);
    compactVarInt(&vnew);
    destroyVarInt(*vout);
    *vout = vnew;
}

// Subtracts two VariableInts, provided that the first one is bigger than the second
void subVarInt(VariableInt *vi1, VariableInt *vi2, VariableInt *vout)
{
    // For optimization purposes, this does not use addVarInt directly.
    VariableInt vnew;
    int vi1len = getNeededLen(vi1);
    vnew.len = vi1len;
    vnew.data = malloc(sizeof(uint64_t) * vi1len);
    uint64_t *vdata = vnew.data;
    uint64_t *v1data = vi1->data;
    uint64_t *v2data = vi2->data;

    bool carry1 = true;
    bool carry = false;
    for (int i = 0; i < vi1len; i++)
    {
        vdata[i] = ~(v2data[i]) + carry1;
        carry1 = !(vdata[i]) && carry1;
        vdata[i] = v1data[i] + vdata[i] + carry;
        carry = vdata[i] < v1data[i]; // Fast way to detect if a carry bit should be set or not
    }

    // Clear out any pointers (if vi1 or vi2 are identical to vout, then they will be destroyed here too)
    destroyVarInt(*vout);
    *vout = vnew;
    //compactVarInt(&vnew);
}

// Used in the division algorithm, gets the "length of bits", not to be confused with the Hamming weight
int bitlenVarInt(const VariableInt *vi)
{
    int i = vi->len - 1;
    for (; (i > 0) && !vi->data[i]; i--)
    {

    }
    return (i << 6) + (unsigned) (8*sizeof (unsigned long long) - __builtin_clzll((vi->data[i])));
}

// Divide two VariableInts, with integer long division (The algorithm used here is derived from Wikipedia's article)
uint64_t divVarInt(VariableInt *vi1, VariableInt *vi2, VariableInt *vout)
{
    int vi1len = getNeededLen(vi1);
    int vi2len = getNeededLen(vi2);
    if (vi2len == 1 && vi2->data[0] == 0)
    {
        fprintf(stderr, "\033[31mError: DIV0\033[0m");
        exit(1);
    }
    VariableInt viNew = variableIntL(vi1len);
    VariableInt viRem = variableInt();
    uint64_t *vi1data = vi1->data;

    // Compute the number of bits in vi1
    for (int i = ((vi1len - 1) << 6) + (unsigned) (8*sizeof (unsigned long long) - __builtin_clzll((vi1->data[vi1len - 1]))) - 1; i >= 0; i--) // bitlenVarInt(vi1) - 1
    {
        lshiftVarInt(&viRem, &viRem, 1);
        int pos = i >> 6;
        int pos2 = i & 0x3f;
        viRem.data[0] ^= ((vi1data[pos] >> (pos2)) ^ viRem.data[0]) & 1ULL; // The setting mechanism has been derived from StackOverflow, and the bit-getting was my own work.
        if (maxVP(vi2, &viRem, vi2len, viRem.len) == &viRem) // maxVP works as a <= in this case
        {
            subVarInt(&viRem, vi2, &viRem);
            viNew.data[pos] |= (1ULL << (pos2));
        }
    }

    destroyVarInt(*vout);
    uint64_t ret = viRem.data[0];
    destroyVarInt(viRem);
    *vout = viNew;
    compactVarInt(vout);

    return ret;
}

uint64_t modVarIntI(VariableInt *vi1, uint64_t modand)
{
    uint64_t ret = 0;
    uint64_t rep = 1;
    for (int i = 0; i < getNeededLen(vi1); i++)
    {
        ret += (vi1->data[i] * rep);
        ret %= modand;
        // Here, we're sneakily getting around the 64-bit limit by re-representing the units in each block!
        rep = ((ULLONG_MAX % modand) * (rep + 1)) % modand;
    }

    return ret;
}