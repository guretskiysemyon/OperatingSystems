/*
 * Author: Semyon Guretskiy
 * Date: 25/09/2023
 */
#include <stdio.h>
#include <stdlib.h>
#include "News.h"


News *create_new(int pr, int ind, int cat)
{
    News *n = (News *)malloc(sizeof(News));
    if (n == NULL)
    {
        printf("Error! Memory allocating\n");
        exit(1);
    }
    n->category = cat;
    n->index = ind;
    n->producer = pr;
    return n;
}