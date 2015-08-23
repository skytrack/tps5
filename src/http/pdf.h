//******************************************************************************
//
// File Name : pdf.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _PDF_H

#define _PDF_H

int job_report_pdf(THREAD_POOL_JOB *job, unsigned char *buffer, size_t buffer_size, const char *font_file_name);

#endif