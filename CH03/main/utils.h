/**
 * @file utils.h
 * @author Bhupinderjeet Singh
 * @brief
 * @version 0.1
 * @date 2023-09-05
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef __UTILS_H__
#define __UTILS_H__

bool utils_replace_characters(char *s, const char ch, const char r);
bool findStringValueForKey(const char *jsonString, const char *key, char *value, size_t bufferSize);

#endif /* __UTILS_H__ */