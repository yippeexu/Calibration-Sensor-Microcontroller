/*
 * DataSet.h
 *
 *  Created on: Jul 8, 2015
 *      Author: root
 */

#ifndef DATASET_H_
#define DATASET_H_

#include "generics.h"
#include "list.h"
#include "Vector.h"

#define LAST_CALIBRATED_COLUMN MAG_THETA

typedef void* void_ptr;

import_header(list, int);
import_header(list, long);
import_header(list, double);
import_header(list, void_ptr);
import_header(list, list(int));

typedef struct {
	double t;
	Vector acl, gyr, mag;
} JoinedData;
import_header(list, JoinedData);
import_header(list, list(JoinedData));


typedef struct {
	double t;
	Vector acl, gyr;
	PolarVector mag;
} CalibratedData;

typedef union {
	JoinedData joined;
	CalibratedData calibrated;
	double data[10];
} Double10;

typedef struct {
	int is_normalized;
	int is_smoothed;
	int len;
	CalibratedData* values;
} CalibratedDataList;

import_header(list, CalibratedDataList);

typedef enum {
	ACL_X, ACL_Y, ACL_Z, GYR_X, GYR_Y, GYR_Z, MAG_R, MAG_PHI, MAG_THETA
} CalibratedColumn;

typedef struct {
	int is_positive_peak;
	double t;
	double value;
} Peak;
import_header(list, Peak);
import_header(list, list(Peak));

typedef struct {
	CalibratedDataList data;
	int ind_start, ind_end;
} NDS;
import_header(list, NDS);

typedef struct {
	NDS data;
	list(Peak) *cols[LAST_CALIBRATED_COLUMN + 1];
} Trial;
import_header(list, Trial);

typedef struct {
	double mu, sigma;
} Distribution;
import_header(list, Distribution);

typedef struct {
	int n_samples;
	list(Distribution) *distributions[LAST_CALIBRATED_COLUMN + 1];
	list(int) *calibration_columns;
	list(list(int)) *calibration_signatures;
} CurveDefinition;

list(JoinedData) dataset_combine_vector4(Vector4* mag, int magl,
		Vector4* gyr, int gryl, Vector4* acl, int acll, double dt);

Vector averageAcl(list(JoinedData) data);
Vector averageMag(list(JoinedData) data);
Vector averageGyr(list(JoinedData) data);
/**
 * Gets the field in the calibrated data struct corresponding to the
 * given column
 */
double *dataset_column_get_field(CalibratedData* data, CalibratedColumn column);
/**
 * Returns a text representation of the given column
 */
char* dataset_column_render(CalibratedColumn column);

double dataset_nds_duration(NDS nds);

void dataset_free_trial(Trial tr);

CalibratedDataList dataset_nds_to_cdl(NDS nds);

#endif /* DATASET_H_ */
