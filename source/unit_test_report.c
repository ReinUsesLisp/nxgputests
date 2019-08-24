#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <errno.h>

#include "unit_test_report.h"

#define APPNAME "nxgputests"
#define EXTRA_CHARS (sizeof(APPNAME) + sizeof(".json") + 1)

FILE* begin_unit_test_report()
{
	FILE* report_file = fopen(APPNAME ".json", "w");
	if (!report_file)
	{
		fprintf(stderr, "Failed to create file \"" APPNAME ".json\"!");
		return NULL;
	}

	fprintf(report_file,
		"{\n\t\"name\": \"" APPNAME ".nro\",\n\t\"results\": [\n");
	return report_file;
}

void unit_test_report(FILE* report_file, char const* test_name, bool pass,
	size_t num_entries, uint32_t const* expected, uint32_t const* results)
{
	if (report_file)
	{
		fprintf(report_file,
				"\t\t{\n\t\t\t\"name\": \"%s\",\n\t\t\t\"pass\": %s\n\t\t},\n",
				test_name, pass ? "true" : "false");
	}

	size_t len = strlen(test_name);
	char* filename = malloc(len + EXTRA_CHARS);
	if (!filename)
	{
		fprintf(stderr, "Out of memory!\n");
		return;
	}

	if (mkdir(APPNAME, 0777) == -1 && errno != EEXIST)
	{
		fprintf(stderr, "Failed to create directory!\n");
		goto release_filename;
	}

	size_t filename_len = (size_t)snprintf(filename, len + EXTRA_CHARS,
		APPNAME "/%s.json", test_name);
	for (size_t i = 0; i < filename_len; ++i)
	{
		if (filename[i] == ' ')
			filename[i] = '_';
		else if (filename[i] == '|')
			filename[i] = 'l';
	}

	FILE* file = fopen(filename, "w");
	if (!file)
	{
		fprintf(stderr, "Failed to create file \"%s\"", filename);
		goto release_filename;
	}

	fprintf(file,
		"{\n"
		"\t\"name\": \"%s\",\n"
		"\t\"pass\": \"%s\",\n"
		"\t\"results\": [\n\t\t", test_name, pass ? "true" : "false");
	for (size_t i = 0; i < num_entries; ++i)
		fprintf(file, "%u, ", results[i]);
	fprintf(file, "\n\t],\n\t\"expected\": [\n\t\t");
	for (size_t i = 0; i < num_entries; ++i)
		fprintf(file, "%u, ", expected[i]);
	fprintf(file, "\n\t],\n}\n");

	fclose(file);
release_filename:
	free(filename);
}

void finish_unit_test_report(FILE* report_file)
{
	if (!report_file)
		return;

	fprintf(report_file, "\t]\n}");
	fclose(report_file);
}