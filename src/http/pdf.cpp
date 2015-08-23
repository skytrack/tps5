//******************************************************************************
//
// File Name : pdf.cpp
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <string.h>
#include <stdio.h>
#include <vector>
#include <math.h>
#include <setjmp.h>
#include "hpdf.h"
#include "json_parse.h"
#include "http.h"
#include "thread_pool.h"
#include "response.h"
#include "api.h"

static void error_handler (HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data)
{
	jmp_buf *env = (jmp_buf *)user_data;

	api_log_printf("[PDF] ERROR: error_no=%04X, detail_no=%u\r\n", (HPDF_UINT)error_no, (HPDF_UINT)detail_no);

    longjmp(*env, 1);
}

HPDF_Page PDF_AddPage(HPDF_Doc pdf, HPDF_Font font, HPDF_REAL font_size, bool bLandscape)
{
	HPDF_Page page = HPDF_AddPage(pdf);

	HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, (bLandscape) ? HPDF_PAGE_LANDSCAPE : HPDF_PAGE_PORTRAIT);
	HPDF_Page_SetLineWidth(page, 1);
	HPDF_Page_SetFontAndSize(page, font, font_size);

	HPDF_Box font_bbox		= HPDF_Font_GetBBox(font);
	HPDF_REAL line_spacing	= font_size * (font_bbox.top - font_bbox.bottom) / 1000;

	HPDF_Page_SetTextLeading(page, line_spacing);

	return page;
}

int PDF_MakeTableReport(JVALUE *value, unsigned char *buffer, size_t *buffer_size, const char *font_file_name, bool bLandscape)
{
    HPDF_Doc	pdf;
    HPDF_Font	font;
    HPDF_Page	page;
	JVALUE *table_key;
	JVALUE *title_key;
	JVALUE *rows_key;
	size_t rows_count = 0;
	jmp_buf		env;

	std::vector<HPDF_REAL> column_width;

	for (table_key = value->value.list_val.first_value; table_key != NULL; table_key = table_key->next) {
	
		if ((table_key->type == JSON_PARSE_VALUE_TYPE_ARRAY)&&(table_key->key_len == 5)&&(memcmp(table_key->key, "table", 5) == 0))
			break;
	}

	if (table_key == NULL)
		return -1;

	for (title_key = value->value.list_val.first_value; title_key != NULL; title_key = title_key->next) {
	
		if ((title_key->type == JSON_PARSE_VALUE_TYPE_STRING)&&(title_key->key_len == 5)&&(memcmp(title_key->key, "title", 5) == 0)) {
			title_key->value.str_val[title_key->str_len] = '\0';
			break;
		}
	}

	for (rows_key = value->value.list_val.first_value; rows_key != NULL; rows_key = rows_key->next) {
	
		if ((rows_key->type == JSON_PARSE_VALUE_TYPE_ARRAY)&&(rows_key->key_len == 4)&&(memcmp(rows_key->key, "rows", 4) == 0)) {

			for (JVALUE *row = rows_key->value.list_val.first_value; row != NULL; row = row->next) {
				
				if (row->type == JSON_PARSE_VALUE_TYPE_STRING) {
					row->value.str_val[row->str_len] = '\0';
					rows_count++;
				}
			}
			break;
		}
	}

	size_t columns_count = 0;

	JVALUE *first_row = table_key->value.list_val.first_value;
	for (JVALUE *cell = first_row->value.list_val.first_value; cell != NULL; cell = cell->next)
		columns_count++;

	if (columns_count == 0)
		return -1;

	column_width.reserve(columns_count);
	for (size_t i = 0; i < columns_count; i++)
		column_width.push_back(0);

	pdf = HPDF_New(error_handler, env);

	if (pdf == NULL) {
		return -1;
	}

	if (setjmp(env)) {
		HPDF_Free(pdf);
		return -1;
    }

	HPDF_SetCompressionMode (pdf, HPDF_COMP_ALL);
	HPDF_UseUTFEncodings(pdf); 

	font = HPDF_GetFont(pdf, HPDF_LoadTTFontFromFile(pdf, font_file_name, HPDF_TRUE), "UTF-8"); 

	page = PDF_AddPage(pdf, font, 12, bLandscape);

	size_t row_count = 0;

	for (JVALUE *row = table_key->value.list_val.first_value; row != NULL; row = row->next) {

		row_count++;

		size_t column = 0;

		for (JVALUE *cell = row->value.list_val.first_value; (cell != NULL) && (column < columns_count); cell = cell->next, column++) {

			if (cell->type == JSON_PARSE_VALUE_TYPE_STRING) {

				cell->value.str_val[cell->str_len] = '\0';
						
				HPDF_REAL cell_width = HPDF_Page_TextWidth(page, (char *)cell->value.str_val) + 1;

				if (cell_width > column_width[column])
					column_width[column] = cell_width;
			}
		}
	}

	HPDF_REAL title_height = 0;

	if (title_key != NULL) {
	
		HPDF_Page_SetFontAndSize(page, font, 16);

		HPDF_Box font_bbox		= HPDF_Font_GetBBox(font);
		title_height			= 16 * (font_bbox.top - font_bbox.bottom) / 1000;

		HPDF_Page_SetFontAndSize(page, font, 12);
	}

	HPDF_REAL cell_padding = 2;
	HPDF_Box table_rect;

	table_rect.top		= HPDF_Page_GetHeight(page) - 50 - title_height - rows_count * HPDF_Page_GetTextLeading(page);
	table_rect.bottom	= 50;
	table_rect.left		= 50;
	table_rect.right	= HPDF_Page_GetWidth(page) - 50;

	HPDF_REAL table_width	= table_rect.right - table_rect.left;
	HPDF_REAL table_height	= table_rect.top - table_rect.bottom;

	HPDF_REAL full_row_height	= cell_padding + HPDF_Page_GetTextLeading(page) + cell_padding;
	size_t rows_per_table		= (size_t)floor(table_height / full_row_height) - 1; // One row for headers

	std::vector<size_t> pages;

	HPDF_REAL content_width = cell_padding + column_width[0] + cell_padding;

	pages.push_back(1);

	for (size_t i = 1; i < columns_count; i++) {

		HPDF_REAL full_column_width = cell_padding + column_width[i] + cell_padding;

		content_width += full_column_width;

		if (content_width > table_width) {
			content_width = cell_padding + column_width[0] + cell_padding;
			pages.push_back(i);
		}
	}

	pages.push_back(columns_count);
	
	size_t pages_count_y = (size_t)ceil((float)row_count / rows_per_table);
	size_t pages_count_x = pages.size();

	for (size_t page_y = 0; page_y < pages_count_y; page_y++) {

		size_t first_row_on_this_page	= page_y * rows_per_table + 1;
		size_t last_row_on_this_page	= first_row_on_this_page + rows_per_table - 1;

		for (size_t page_x = 1; page_x < pages_count_x; page_x++) {

			size_t first_column_on_this_page	= pages[page_x - 1];
			size_t last_column_on_this_page		= pages[page_x] - 1;

			if (page == NULL)
				page = PDF_AddPage(pdf, font, 12, bLandscape);
			
			HPDF_REAL y = HPDF_Page_GetHeight(page) - 50;

			if (title_key != NULL) {

				HPDF_Page_SetFontAndSize(page, font, 16);

				HPDF_Page_BeginText(page);
				HPDF_Page_TextOut(page, (HPDF_Page_GetWidth(page) / 2) - HPDF_Page_TextWidth(page, (char *)title_key->value.str_val) / 2, y,  (char *)title_key->value.str_val);
				HPDF_Page_EndText(page);

				HPDF_Page_SetFontAndSize(page, font, 12);

				y -= title_height;
			}

			if (rows_key != NULL) {

				size_t current_row = 0;

				for (JVALUE *row = rows_key->value.list_val.first_value; row != NULL; row = row->next) {
				
					if (row->type == JSON_PARSE_VALUE_TYPE_STRING) {

						HPDF_Page_BeginText(page);
						HPDF_Page_TextOut(page, 50, y, (char *)row->value.str_val);
						HPDF_Page_EndText(page);

						y -= HPDF_Page_GetTextLeading(page);
					}
				}
			}

			HPDF_REAL x = table_rect.left;
			y = table_rect.top;

			size_t current_row	= 0;
			size_t current_column;

			for (JVALUE *row = table_key->value.list_val.first_value; row != NULL; row = row->next, current_row++) {

				if ((current_row == 0)||((current_row >= first_row_on_this_page)&&(current_row <= last_row_on_this_page))) {

					current_column = 0;

					x = table_rect.left;

					for (JVALUE *cell = row->value.list_val.first_value; cell != NULL; cell = cell->next, current_column++) {

 						if ((current_column == 0)||((current_column >= first_column_on_this_page)&&(current_column <= last_column_on_this_page))) {

							HPDF_Page_BeginText(page);
							HPDF_Page_TextRect(page, x + cell_padding, y - cell_padding, x + cell_padding + column_width[current_column], y - full_row_height + cell_padding,  (char *)cell->value.str_val, HPDF_TALIGN_LEFT, NULL);
							HPDF_Page_EndText(page);

							x += column_width[current_column] + cell_padding * 2;
						}
					}

					HPDF_Page_MoveTo(page, table_rect.left, y);
					HPDF_Page_LineTo(page, x, y);
					HPDF_Page_Stroke(page);

					y -= full_row_height;
				}
			}

			HPDF_Page_MoveTo(page, table_rect.left, y);
			HPDF_Page_LineTo(page, x, y);
			HPDF_Page_Stroke(page);

			current_column = 0;
			x = table_rect.left;

			for (JVALUE *cell = first_row->value.list_val.first_value; cell != NULL; cell = cell->next, current_column++) {

 				if ((current_column == 0)||((current_column >= first_column_on_this_page)&&(current_column <= last_column_on_this_page))) {

					HPDF_Page_MoveTo(page, x, table_rect.top);
					HPDF_Page_LineTo(page, x, y);
					HPDF_Page_Stroke(page);

					x += column_width[current_column] + cell_padding * 2;
				}
			}

			HPDF_Page_MoveTo(page, x, table_rect.top);
			HPDF_Page_LineTo(page, x, y);
			HPDF_Page_Stroke(page);

			page = NULL;
		}
	}

	HPDF_SaveToStream(pdf);
	
	HPDF_UINT32 size = HPDF_GetStreamSize(pdf);

	if (size <= *buffer_size) {
		HPDF_ReadFromStream(pdf, buffer, &size);
		*buffer_size = size;
	}
	else {
		HPDF_Free (pdf);
		return -1;
	}

	HPDF_Free(pdf);

	return 0;
}

int PDF_MakeSectionReport(JVALUE *value, unsigned char *buffer, size_t *buffer_size, const char *font_file_name, bool bLandscape)
{
    HPDF_Doc	pdf;
    HPDF_Font	font;
    HPDF_Page	page;
	jmp_buf		env;

	pdf = HPDF_New(error_handler, env);

	if (pdf == NULL) {
		return -1;
	}

	if (setjmp(env)) {
		HPDF_Free(pdf);
		return -1;
    }

	HPDF_SetCompressionMode (pdf, HPDF_COMP_ALL);
	HPDF_UseUTFEncodings(pdf); 

	font = HPDF_GetFont(pdf, HPDF_LoadTTFontFromFile(pdf, font_file_name, HPDF_TRUE), "UTF-8"); 

	page = PDF_AddPage(pdf, font, 12, bLandscape);

	HPDF_REAL y = HPDF_Page_GetHeight(page) - 50;

	// Нахожу название отчета
	for (JVALUE *key = value->value.list_val.first_value; key != NULL; key = key->next) {

		if ((key->type == JSON_PARSE_VALUE_TYPE_STRING)&&(key->key_len == 5)&&(memcmp(key->key, "title", 5) == 0)) {
		
			key->value.str_val[key->str_len] = '\0';

			HPDF_Page_SetFontAndSize(page, font, 16);

			HPDF_Page_BeginText(page);
			HPDF_Page_TextOut(page, (HPDF_Page_GetWidth(page) / 2) - HPDF_Page_TextWidth(page, (char *)key->value.str_val) / 2, y,  (char *)key->value.str_val);
			HPDF_Page_EndText(page);

			HPDF_Box font_bbox		= HPDF_Font_GetBBox(font);
			y -= 16 * (font_bbox.top - font_bbox.bottom) / 1000;

			HPDF_Page_SetFontAndSize(page, font, 12);

			break;
		}
	}

	y -= HPDF_Page_GetTextLeading(page);

	// Нахожу текстовые строки отчета
	for (JVALUE *key = value->value.list_val.first_value; key != NULL; key = key->next) {
	
		if ((key->type == JSON_PARSE_VALUE_TYPE_ARRAY)&&(key->key_len == 4)&&(memcmp(key->key, "rows", 4) == 0)) {
		
			for (JVALUE *row = key->value.list_val.first_value; row != NULL; row = row->next) {

				row->value.str_val[row->str_len] = '\0';

				HPDF_Page_BeginText(page);
				HPDF_Page_TextOut(page, 50, y, (char *)row->value.str_val);
				HPDF_Page_EndText(page);

				y -= HPDF_Page_GetTextLeading(page);
			}

			break;
		}
	}

	// Нахожу секции
	for (JVALUE *key = value->value.list_val.first_value; key != NULL; key = key->next) {
	
		if ((key->type == JSON_PARSE_VALUE_TYPE_ARRAY)&&(key->key_len == 8)&&(memcmp(key->key, "sections", 8) == 0)) {
		
			// Иду по каждой секции
			for (JVALUE *section = key->value.list_val.first_value; section != NULL; section = section->next) {

				y -= HPDF_Page_GetTextLeading(page);

				// Нахожу заголовок секции
				for (JVALUE *section_key = section->value.list_val.first_value; section_key != NULL; section_key = section_key->next) {

					if ((section_key->type == JSON_PARSE_VALUE_TYPE_STRING)&&(section_key->key_len == 5)&&(memcmp(section_key->key, "title", 5) == 0)) {

						section_key->value.str_val[section_key->str_len] = '\0';

						HPDF_Page_SetFontAndSize(page, font, 14);

						HPDF_Page_BeginText(page);
						HPDF_Page_TextOut(page, (HPDF_Page_GetWidth(page) / 2) - HPDF_Page_TextWidth(page, (char *)section_key->value.str_val) / 2, y,  (char *)section_key->value.str_val);
						HPDF_Page_EndText(page);

						HPDF_Box font_bbox		= HPDF_Font_GetBBox(font);
						y -= 14 * (font_bbox.top - font_bbox.bottom) / 1000;

						if (y < 50) {

							page = PDF_AddPage(pdf, font, 12, bLandscape);

							y = HPDF_Page_GetHeight(page) - 50;
						}

						HPDF_Page_SetFontAndSize(page, font, 12);

						break;
					}
				}

				y -= HPDF_Page_GetTextLeading(page);

				// Нахожу объекты секции
				for (JVALUE *section_key = section->value.list_val.first_value; section_key != NULL; section_key = section_key->next) {

					if ((section_key->type == JSON_PARSE_VALUE_TYPE_ARRAY)&&(section_key->key_len == 7)&&(memcmp(section_key->key, "objects", 7) == 0)) {

						// Прохожусь по объектам секции
						for (JVALUE *object_key = section_key->value.list_val.first_value; object_key != NULL; object_key = object_key->next) {

							// Нахожу тип объекта
							for (JVALUE *type_key = object_key->value.list_val.first_value; type_key != NULL; type_key = type_key->next) {

								if ((type_key->type == JSON_PARSE_VALUE_TYPE_STRING)&&(type_key->key_len == 4)&&(memcmp(type_key->key, "type", 4) == 0)) {

									if ((type_key->str_len == 3)&&(memcmp(type_key->value.str_val, "row", 3) == 0)) {

										// Прохожусь по объектам секции
										for (JVALUE *data_key = object_key->value.list_val.first_value; data_key != NULL; data_key = data_key->next) {
											
											if ((data_key->type == JSON_PARSE_VALUE_TYPE_STRING)&&(data_key->key_len == 4)&&(memcmp(data_key->key, "data", 4) == 0)) {

												data_key->value.str_val[data_key->str_len] = '\0';

												HPDF_Page_BeginText(page);
												HPDF_Page_TextOut(page, 50, y, (char *)data_key->value.str_val);
												HPDF_Page_EndText(page);

												y -= HPDF_Page_GetTextLeading(page);

												if (y < 50) {

													page = PDF_AddPage(pdf, font, 12, bLandscape);

													y = HPDF_Page_GetHeight(page) - 50;
												}

												break;
											}
										}
									}

									if ((type_key->str_len == 5)&&(memcmp(type_key->value.str_val, "table", 5) == 0)) {

										HPDF_REAL cell_padding = 2;
										HPDF_REAL full_row_height	= cell_padding + HPDF_Page_GetTextLeading(page) + cell_padding;
										HPDF_REAL table_top = y;

										// Прохожусь по объектам секции
										for (JVALUE *data_key = object_key->value.list_val.first_value; data_key != NULL; data_key = data_key->next) {
											
											if ((data_key->type == JSON_PARSE_VALUE_TYPE_ARRAY)&&(data_key->key_len == 4)&&(memcmp(data_key->key, "data", 4) == 0)) {

												size_t row_count = 0;
												std::vector<HPDF_REAL> column_width;

												for (JVALUE *row = data_key->value.list_val.first_value; row != NULL; row = row->next) {

													row_count++;

													size_t column = 0;

													for (JVALUE *cell = row->value.list_val.first_value; cell != NULL; cell = cell->next, column++) {

														if (cell->type == JSON_PARSE_VALUE_TYPE_STRING) {

															cell->value.str_val[cell->str_len] = '\0';
						
															HPDF_REAL cell_width = HPDF_Page_TextWidth(page, (char *)cell->value.str_val) + 1;

															if (column_width.size() == column) {
																column_width.push_back(cell_width);
															}
															else {
																if (cell_width > column_width[column]) {
																	column_width[column] = cell_width;
																}
															}
														}
													}
												}

												size_t current_column;
												HPDF_REAL x;
												bool table_closed = false;

												for (JVALUE *row = data_key->value.list_val.first_value; row != NULL; row = row->next) {

													x = 50;

													current_column = 0;

													for (JVALUE *cell = row->value.list_val.first_value; cell != NULL; cell = cell->next, current_column++) {

 														HPDF_Page_BeginText(page);
														HPDF_Page_TextRect(page, x + cell_padding, y - cell_padding, x + cell_padding + column_width[current_column], y - full_row_height + cell_padding,  (char *)cell->value.str_val, HPDF_TALIGN_LEFT, NULL);
														HPDF_Page_EndText(page);

														x += column_width[current_column] + cell_padding * 2;
													}
												
													y -= full_row_height;

													HPDF_Page_MoveTo(page, 50, y);
													HPDF_Page_LineTo(page, x, y);
													HPDF_Page_Stroke(page);

													if (y < 50) {

														current_column = 0;
														x = 50;

														for (JVALUE *cell = data_key->value.list_val.first_value->value.list_val.first_value; cell != NULL; cell = cell->next, current_column++) {

 															HPDF_Page_MoveTo(page, x, table_top);
															HPDF_Page_LineTo(page, x, y);
															HPDF_Page_Stroke(page);

															x += column_width[current_column] + cell_padding * 2;
														}

 														HPDF_Page_MoveTo(page, x, table_top);
														HPDF_Page_LineTo(page, x, y);
														HPDF_Page_Stroke(page);

														HPDF_Page_MoveTo(page, 50, table_top);
														HPDF_Page_LineTo(page, x, table_top);
														HPDF_Page_Stroke(page);

														page = PDF_AddPage(pdf, font, 12, bLandscape);

														y = HPDF_Page_GetHeight(page) - 50;

														if (row->next != NULL) {
															HPDF_Page_MoveTo(page, 50, y);
															HPDF_Page_LineTo(page, x, y);
															HPDF_Page_Stroke(page);
														}
														else {
															table_closed = true;
														}

														table_top = y;
													}

												}

												if (!table_closed) {

													HPDF_Page_MoveTo(page, 50, table_top);
													HPDF_Page_LineTo(page, x, table_top);
													HPDF_Page_Stroke(page);

													current_column = 0;
													x = 50;

													for (JVALUE *cell = data_key->value.list_val.first_value->value.list_val.first_value; cell != NULL; cell = cell->next, current_column++) {

 														HPDF_Page_MoveTo(page, x, table_top);
														HPDF_Page_LineTo(page, x, y);
														HPDF_Page_Stroke(page);

														x += column_width[current_column] + cell_padding * 2;
													}

 													HPDF_Page_MoveTo(page, x, table_top);
													HPDF_Page_LineTo(page, x, y);
													HPDF_Page_Stroke(page);

													y -= full_row_height;

													if (y < 50) {

														page = PDF_AddPage(pdf, font, 12, bLandscape);

														y = HPDF_Page_GetHeight(page) - 50;
													}
												}

												break;
											}
										}
									}

									break;
								}
							}
						}

						break;
					}
				}

			}
		}
	}

	HPDF_SaveToStream(pdf);
	
	HPDF_UINT32 size = HPDF_GetStreamSize(pdf);

	if (size <= *buffer_size) {
		HPDF_ReadFromStream(pdf, buffer, &size);
		*buffer_size = size;
	}
	else {
		HPDF_Free (pdf);
		return -1;
	}

	HPDF_Free(pdf);

	return 0;
}

int decodeURIComponent(unsigned char *sSource, size_t length) 
{	
	unsigned char *sDest = sSource;
	unsigned char *sEnd = sSource + length;

    int nLength;
    for (nLength = 0; sSource != sEnd; nLength++) {
        if (*sSource == '%' && sSource[1] && sSource[2] && isxdigit(sSource[1]) && isxdigit(sSource[2])) {
            sSource[1] -= sSource[1] <= '9' ? '0' : (sSource[1] <= 'F' ? 'A' : 'a')-10;
            sSource[2] -= sSource[2] <= '9' ? '0' : (sSource[2] <= 'F' ? 'A' : 'a')-10;
            sDest[nLength] = 16 * sSource[1] + sSource[2];
            sSource += 3;
            continue;
        }
		if (*sSource == '+') {
			sDest[nLength] = ' ';
			*sSource++;
			continue;
		}
        sDest[nLength] = *sSource++;
    }
    sDest[nLength] = '\0';
    return nLength;
}

int job_report_pdf(THREAD_POOL_JOB *job, unsigned char *buffer, size_t buffer_size, const char *font_file_name)
{
	const char headers_ka[] = "HTTP/1.1 200 OK\r\n"
					"Content-Type: application/pdf\r\n"
					"Server: attiny2313\r\n"
					"Connection: Keep-alive\r\n"					
					"Content-Length:              \r\n\r\n";

	const char headers_close[] = "HTTP/1.1 200 OK\r\n"
					"Content-Type: application/pdf\r\n"
					"Server: attiny2313\r\n"
					"Connection: Close\r\n"					
					"Content-Length:              \r\n\r\n";

	JVALUE *value;


	size_t json_size = decodeURIComponent(job->buffer, job->buffer_size);

	if (memcmp(job->buffer, "report=", 7)) {
		response_fail_with_message(buffer, &buffer_size, (unsigned char *)"no report field on input", 24, job->session->zero_init.keep_alive > 0);
		api_send_tcp(job->session, buffer, buffer_size);
		return 0;
	}

	json_size -= 7;

	void *parser = json_parse_init();

	size_t data_size = buffer_size;
	if ((value = json_parse_parse(parser, job->buffer + 7, json_size, buffer, &data_size)) != NULL) {

		if (value->type == JSON_PARSE_VALUE_TYPE_OBJECT) {
		
			unsigned char *response = buffer + data_size;
			unsigned char *pdf;

			if (job->session->zero_init.keep_alive > 0) {
				memcpy(response, headers_ka, sizeof(headers_ka) - 1);
				pdf = response + sizeof(headers_ka) - 1;
			}
			else {
				memcpy(response, headers_close, sizeof(headers_close) - 1);
				pdf = response + sizeof(headers_close) - 1;
			}

			unsigned char *content_length_ptr = pdf - 4;	
			size_t content_length = buffer_size - (pdf - buffer);

			bool bTable = false;
			bool bLandscape = false;
			for (JVALUE *table_key = value->value.list_val.first_value; table_key != NULL; table_key = table_key->next) {
	
				if ((table_key->type == JSON_PARSE_VALUE_TYPE_ARRAY)&&(table_key->key_len == 5)&&(memcmp(table_key->key, "table", 5) == 0)) {
					bTable = true;
				}
				if ((table_key->type == JSON_PARSE_VALUE_TYPE_BOOLEAN)&&(table_key->key_len == 9)&&(memcmp(table_key->key, "landscape", 9) == 0)&&(table_key->value.bool_val)) {
					bLandscape = true;
				}
			}

			int result;

			if (bTable)
				result = PDF_MakeTableReport(value, pdf, &content_length, font_file_name, bLandscape);
			else
				result = PDF_MakeSectionReport(value, pdf, &content_length, font_file_name, bLandscape);

			if (result == 0) {

				size_t pdf_size = content_length;

				json_parse_destroy(parser);
		
				do {
					*--content_length_ptr = '0' + content_length % 10;
					content_length /= 10;
				} while (content_length != 0);

				api_send_tcp(job->session, response, pdf_size + (pdf - response));

				return 0;
			}
		}
	}
	else {
		unsigned char *error = json_parse_get_error(parser, job->buffer + 7, json_size);
		size_t len = strlen((char *)error);

		response_fail_with_message(buffer, &buffer_size, error, len, job->session->zero_init.keep_alive > 0);
		json_parse_free_error(parser);

		api_send_tcp(job->session, buffer, buffer_size);

		json_parse_destroy(parser);

		return 0;
	}

	json_parse_destroy(parser);

	return 500;
}