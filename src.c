/*
 * src.c  - calculate source code changes
 *
 * Copyright (C) 2008 Ali Gholami Rudi
 *
 * Currently inserts and deletes don't span many lines; that makes
 * src_insert(), src_delete() and src_print_diffs() much simpler.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "src.h"
#include "utils.h"

#define MAXLINELEN	1024

struct line {
	char *line;
	struct line *next;
};

struct src *src_from_file(char *filename)
{
	char line[MAXLINELEN];
	struct line *tail = NULL;
	int count = 0;
	FILE *input = fopen(filename, "r");
	struct src *result = xmalloc(sizeof(struct src));
	int i;

	while (fgets(line, MAXLINELEN, input)) {
		struct line *cur = xmalloc(sizeof(struct line));
		cur->line = xmalloc(strlen(line) + 1);
		strcpy(cur->line, line);
		cur->next = tail;
		tail = cur;
		count++;
	}
	result->count = count;
	result->lines = xmalloc(sizeof(char *) * count);
	result->offsets = xmalloc(sizeof(long) * count);
	result->invalid_from = 0;
	for (i = count - 1; i >= 0; i--) {
		struct line *next = tail->next;
		result->lines[i] = tail->line;
		free(tail);
		tail = next;
	}
	return result;
}

static long line_offset(struct src *src, int line)
{
	if (src->invalid_from <= line) {
		int i;
		for (i = src->invalid_from; i <= line; i++)
			if (!i)
				src->offsets[i] = 0;
			else
				src->offsets[i] = src->offsets[i - 1] +
					strlen(src->lines[i - 1]);
		src->invalid_from = line + 1;
	}
	return src->offsets[line];
}

struct src *src_copy(struct src *src)
{
	struct src *result = xmalloc(sizeof(struct src));
	int i;
	result->count = src->count;
	result->lines = xmalloc(sizeof(char *) * src->count);
	result->offsets = xmalloc(sizeof(long) * src->count);
	for (i = 0; i < src->count; i++) {
		result->lines[i] = xmalloc(strlen(src->lines[i]) + 1);
		strcpy(result->lines[i], src->lines[i]);
	}
	result->invalid_from = 0;
	return result;
}

void src_free(struct src *src)
{
	int i;
	for (i = 0; i < src->count; i++)
		free(src->lines[i]);
	free(src->lines);
	free(src->offsets);
	free(src);
}

static int find_line(struct src *src, long offset)
{
	int start = 0;
	int end = src->count - 1;
	while (start < end) {
		long mid = (start + end) / 2;
		if (offset < line_offset(src, mid))
			end = mid - 1;
		else if (mid < end && line_offset(src, mid + 1) <= offset)
			start = mid + 1;
		else
			return mid;
	}
	return start;
}

void src_delete(struct src *src, long from, long to)
{
	int lineno = find_line(src, from);
	char *newline = xmalloc(strlen(src->lines[lineno]) - (to - from) + 1);
	long before = from - line_offset(src, lineno);
	long after = to - line_offset(src, lineno);
	memcpy(newline, src->lines[lineno], before);
	strcpy(newline + before, src->lines[lineno] + after);
	free(src->lines[lineno]);
	src->lines[lineno] = newline;
	src->invalid_from = lineno;
}

void src_insert(struct src *src, long offset, char *s)
{
	int lineno = find_line(src, offset);
	char *newline = xmalloc(strlen(src->lines[lineno]) + strlen(s) + 1);
	long point = offset - line_offset(src, lineno);
	memcpy(newline, src->lines[lineno], point);
	strcpy(newline + point, s);
	strcpy(newline + point + strlen(s), src->lines[lineno] + point);
	free(src->lines[lineno]);
	src->lines[lineno] = newline;
	src->invalid_from = lineno;
}

static void print_diff_header(char *filename)
{
	printf("diff --git a/%s b/%s\n--- a/%s\n+++ b/%s\n",
	       filename, filename, filename, filename);
}

static int find_start(int start, char *diffs, int size)
{
	int cur;
	for (cur = start; cur < size; cur++)
		if (diffs[cur])
			return cur - 3 < start ? start : cur - 3;
	return -1;
}

static int find_end(int start, char *diffs, int size)
{
	int cur;
	int ones = 0;
	for (cur = start; cur < size; cur++) {
		ones++;
		if (diffs[cur])
			ones = 0;
		if (ones == 7)
			return cur - 3 < start ? start : cur - 3;
	}
	return size;
}

static void print_hunk(struct src *src1, struct src *src2,
		       char *diffs, int start, int end)
{
	int cur1 = start, cur2 = start;
	int len = end - start;
	if (len > 1)
		printf("@@ -%d,%d +%d,%d @@\n", start + 1, len,
		       start + 1, len);
	else
		printf("@@ -%d +%d @@\n", start + 1, start + 1);
	for (; cur1 < end; cur1++) {
		if (diffs[cur1])
			printf("-%s", src1->lines[cur1]);
		if (!diffs[cur1] || cur1 == end - 1) {
			for (; cur2 <= cur1; cur2++) {
				if (diffs[cur2])
					printf("+%s", src2->lines[cur2]);
				else
					printf(" %s", src2->lines[cur2]);
			}
		}
	}
}

static int init_diffs(struct src *src1, struct src *src2, char *diffs)
{
	int differs = 0;
	int i;
	for (i = 0; i < src1->count; i++) {
		diffs[i] = strcmp(src1->lines[i], src2->lines[i]);
		if (diffs[i])
			differs = 1;
	}
	return differs;
}

void src_print_diffs(struct src *src1, struct src *src2, char *filename)
{
	char *diffs = xmalloc(src1->count);
	int start = 0, end = 0;
	if (init_diffs(src1, src2, diffs)) {
		print_diff_header(filename);
		while ((start = find_start(end, diffs, src1->count)) >= 0) {
			end = find_end(start, diffs, src1->count);
			print_hunk(src1, src2, diffs, start, end);
		}
	}
	free(diffs);
}
