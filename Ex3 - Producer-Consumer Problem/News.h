/*
 * Author: Semyon Guretskiy
 * Date: 25/09/2023
 */

#ifndef EX3_NEWS_H
#define EX3_NEWS_H


typedef struct
{
    int producer;
    int index;
    int category;
} News;

/*
 * create_new - Create a new News object with the given parameters.
 * 
 * Parameters:
 *  int pr - Producer ID
 *  int ind - News index
 *  int cat - Category
 * 
 * Return:
 *  News* - Pointer to the newly created News object
 */
News *create_new(int pr, int ind, int cat);

#endif //EX3_NEWS_H
