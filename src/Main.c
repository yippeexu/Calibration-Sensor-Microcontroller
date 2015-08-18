/*
 * Main.c
 *
 *  Created on: Aug 18, 2015
 *      Author: root
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "analysis_peakfind.h"
#include "analysis_preprocessing.h"
#include "analysis_segmentation.h"
#include "analysis_trial_separation.h"
#include "Controller.h"
#include "DataSet.h"
#include "IO.h"
#include "list.h"
#include "Utils.h"

typedef struct {
	list(Trial)* data;
	list(void_ptr)* to_free_on_exit;
} CleanableTrialList;

CleanableTrialList read_single_dataset(char* dir, char* c_readable) {
	list(JoinedData) joined = io_read_joined_dataset(c_readable);
	CalibratedDataList data = analysis_calibrate(joined);
	free(joined.values);
	analysis_smooth(&data);
	analysis_normalize(&data);
	list(NDS)* split = analysis_split_data(&data, 20, .4);
	list(Trial)* trials = analysis_peak_find_all(split, 2);
	char* normed = utils_concat(dir, "/normed.csv");
	io_write_calibrated_data(normed, data);
	free(normed);
	list_free_NDS(split);
	list(void_ptr)* to_free = list_new_void_ptr();
	list_add_void_ptr(to_free, data.values);
	CleanableTrialList ctl = { .data = trials, .to_free_on_exit = to_free };
	return ctl;
}

CleanableTrialList read_all_datasets(char* dir) {
	char* c_readable = utils_concat(dir, "/C-readable.csv");
	if (access(c_readable, F_OK) != -1) {
		CleanableTrialList ctl = read_single_dataset(dir, c_readable);
		free(c_readable);
		return ctl;
	}
	free(c_readable);
	CleanableTrialList general = { .data = list_new_Trial(), .to_free_on_exit =
			list_new_void_ptr() };
	void read_dir(char* lcl_dir) {
		char* cread = utils_concat(lcl_dir, "/C-readable.csv");
		if (access(cread, F_OK) == -1)
			return;
		CleanableTrialList current = read_single_dataset(lcl_dir, cread);
		int i;
		for (i = 0; i < current.data->size; i++) {
			list_add_Trial(general.data, current.data->values[i]);
		}
		list_free_Trial(current.data);
		for (i = 0; i < current.to_free_on_exit->size; i++) {
			list_add_void_ptr(general.to_free_on_exit,
					current.to_free_on_exit->values[i]);
		}
		list_free_void_ptr(current.to_free_on_exit);
	}
	foreach_in_dir(dir, read_dir);
	return general;
}

void process_single_content_folder(char* dir) {
	printf("Attempting to Calculate Peaks for %s\n", dir);
	CleanableTrialList read = read_all_datasets(dir);
	list(Trial)* trials = read.data;
	char* psplit = utils_concat(dir, "/peak_split.csv");
	char* output = utils_concat(dir, "/output.csv");
	io_write_normalized_data_segment_list(psplit, output, trials);
	free(psplit);
	free(output);
	analysis_scale_by_peaks(trials, 2);
	char* psplit_stretched = utils_concat(dir, "/split_stretched.csv");
	char* output_stretched = utils_concat(dir, "/output_stretched.csv");
	io_write_normalized_data_segment_list(psplit_stretched, output_stretched,
			trials);
	free(psplit_stretched);
	free(output_stretched);
	printf("Written all files\n");
	int i;
	for (i = 0; i < trials->size; i++) {
		int j;
		for (j = 0; j <= LAST_CALIBRATED_COLUMN; j++) {
			list_free_Peak(trials->values[i].cols[j]);
		}
	}
	list_free_Trial(trials);
	for (i = 0; i < read.to_free_on_exit->size; i++) {
		free(read.to_free_on_exit->values[i]);
	}
	list_free_void_ptr(read.to_free_on_exit);
}

int main() {
	list(JoinedData) calibration =
			io_read_joined_dataset(
					"/home/kavi/Dropbox/workspaces/C/Magnetometer Processor/calibration.csv");
	cntrl_calibrate(calibration);
	free(calibration.values);
//	char* dir_string =
//			"/home/kavi/Dropbox/workspaces/C/Magnetometer Processor/data";
//	foreach_in_dir(dir_string, process_single_content_folder);
	char* path =
			"/home/kavi/Dropbox/workspaces/C/Magnetometer Processor/data/doorknob-good";
	char* pathcpy = malloc(strlen(path) + 1);
	strcpy(pathcpy, path);
	process_single_content_folder(pathcpy);
	free(pathcpy);
	printf("Completed Successfully");
	return 0;
}
