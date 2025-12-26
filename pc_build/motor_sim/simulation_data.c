/**
 * @file simulation_data.c
 * @brief Data I/O implementation for simulation results
 */

#include "simulation_data.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

// File handles
static FILE *write_file = NULL;
static FILE *read_file = NULL;
static sim_data_format_e current_format = SIM_FORMAT_CSV;
static bool header_written = false;

// Detect format from filename
static sim_data_format_e detect_format(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (ext == NULL) return SIM_FORMAT_CSV;
    
    if (strcmp(ext, ".bin") == 0 || strcmp(ext, ".dat") == 0) {
        return SIM_FORMAT_BINARY;
    } else if (strcmp(ext, ".json") == 0) {
        return SIM_FORMAT_JSON;
    }
    return SIM_FORMAT_CSV;
}

int sim_data_open_write(const char *filename) {
    if (write_file != NULL) {
        fclose(write_file);
    }
    
    current_format = detect_format(filename);
    header_written = false;
    
    if (current_format == SIM_FORMAT_BINARY) {
        write_file = fopen(filename, "wb");
    } else {
        write_file = fopen(filename, "w");
    }
    
    if (write_file == NULL) {
        return -1;
    }
    
    return 0;
}

static void write_csv_header(void) {
    if (write_file == NULL || header_written) return;
    
    fprintf(write_file, "time,id,iq,vd,vq,theta_e,omega_e,omega_m,torque\n");
    header_written = true;
}

int sim_data_write_record(sim_data_record_t *rec) {
    if (write_file == NULL || rec == NULL) return -1;
    
    if (current_format == SIM_FORMAT_CSV) {
        if (!header_written) {
            write_csv_header();
        }
        fprintf(write_file, "%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n",
                rec->time, rec->id, rec->iq, rec->vd, rec->vq,
                rec->theta_e, rec->omega_e, rec->omega_m, rec->torque);
    } else if (current_format == SIM_FORMAT_BINARY) {
        fwrite(rec, sizeof(sim_data_record_t), 1, write_file);
    }
    
    return 0;
}

int sim_data_write_record_ext(sim_data_record_ext_t *rec) {
    if (write_file == NULL || rec == NULL) return -1;
    
    if (current_format == SIM_FORMAT_CSV) {
        if (!header_written) {
            fprintf(write_file, "time,id,iq,vd,vq,theta_e,omega_e,omega_m,torque,"
                    "ia,ib,ic,va,vb,vc,p_in,p_out,p_loss,temp,mod_d,mod_q\n");
            header_written = true;
        }
        fprintf(write_file, 
                "%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,"
                "%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.2f,%.6f,%.6f\n",
                rec->base.time, rec->base.id, rec->base.iq, 
                rec->base.vd, rec->base.vq,
                rec->base.theta_e, rec->base.omega_e, 
                rec->base.omega_m, rec->base.torque,
                rec->ia, rec->ib, rec->ic,
                rec->va, rec->vb, rec->vc,
                rec->p_in, rec->p_out, rec->p_loss,
                rec->temp_motor, rec->mod_d, rec->mod_q);
    } else if (current_format == SIM_FORMAT_BINARY) {
        fwrite(rec, sizeof(sim_data_record_ext_t), 1, write_file);
    }
    
    return 0;
}

void sim_data_close(void) {
    if (write_file != NULL) {
        fclose(write_file);
        write_file = NULL;
    }
    header_written = false;
}

int sim_data_open_read(const char *filename) {
    if (read_file != NULL) {
        fclose(read_file);
    }
    
    current_format = detect_format(filename);
    
    if (current_format == SIM_FORMAT_BINARY) {
        read_file = fopen(filename, "rb");
    } else {
        read_file = fopen(filename, "r");
    }
    
    if (read_file == NULL) {
        return -1;
    }
    
    // Skip CSV header
    if (current_format == SIM_FORMAT_CSV) {
        char line[512];
        if (fgets(line, sizeof(line), read_file) == NULL) {
            fclose(read_file);
            read_file = NULL;
            return -1;
        }
    }
    
    // Count records for binary
    if (current_format == SIM_FORMAT_BINARY) {
        fseek(read_file, 0, SEEK_END);
        long size = ftell(read_file);
        fseek(read_file, 0, SEEK_SET);
        return (int)(size / sizeof(sim_data_record_t));
    }
    
    return 0;  // Unknown count for CSV
}

int sim_data_read_record(sim_data_record_t *rec) {
    if (read_file == NULL || rec == NULL) return -1;
    
    if (current_format == SIM_FORMAT_CSV) {
        char line[512];
        if (fgets(line, sizeof(line), read_file) == NULL) {
            return 1;  // EOF
        }
        
        int n = sscanf(line, "%f,%f,%f,%f,%f,%f,%f,%f,%f",
                       &rec->time, &rec->id, &rec->iq, 
                       &rec->vd, &rec->vq,
                       &rec->theta_e, &rec->omega_e, 
                       &rec->omega_m, &rec->torque);
        if (n != 9) return -1;
        
    } else if (current_format == SIM_FORMAT_BINARY) {
        size_t n = fread(rec, sizeof(sim_data_record_t), 1, read_file);
        if (n != 1) return 1;  // EOF
    }
    
    return 0;
}

int sim_data_read_all(sim_data_record_t *records, int max_records) {
    if (read_file == NULL || records == NULL) return -1;
    
    int count = 0;
    while (count < max_records) {
        int result = sim_data_read_record(&records[count]);
        if (result != 0) break;
        count++;
    }
    
    return count;
}

void sim_data_close_read(void) {
    if (read_file != NULL) {
        fclose(read_file);
        read_file = NULL;
    }
}

int sim_data_export_csv(const char *input_file, const char *output_file) {
    int count = sim_data_open_read(input_file);
    if (count < 0) return -1;
    
    FILE *out = fopen(output_file, "w");
    if (out == NULL) {
        sim_data_close_read();
        return -1;
    }
    
    fprintf(out, "time,id,iq,vd,vq,theta_e,omega_e,omega_m,torque\n");
    
    sim_data_record_t rec;
    while (sim_data_read_record(&rec) == 0) {
        fprintf(out, "%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n",
                rec.time, rec.id, rec.iq, rec.vd, rec.vq,
                rec.theta_e, rec.omega_e, rec.omega_m, rec.torque);
    }
    
    fclose(out);
    sim_data_close_read();
    
    return 0;
}

int sim_data_calc_stats(sim_data_record_t *records, int count,
                        int field_offset, sim_data_stats_t *stats) {
    if (records == NULL || stats == NULL || count <= 0) return -1;
    
    // Get pointer to field using offset
    float *first_value = (float*)((char*)&records[0] + field_offset);
    
    stats->min_value = *first_value;
    stats->max_value = *first_value;
    stats->mean_value = 0.0f;
    stats->rms_value = 0.0f;
    
    float sum = 0.0f;
    float sum_sq = 0.0f;
    
    for (int i = 0; i < count; i++) {
        float *value = (float*)((char*)&records[i] + field_offset);
        float v = *value;
        
        if (v < stats->min_value) stats->min_value = v;
        if (v > stats->max_value) stats->max_value = v;
        
        sum += v;
        sum_sq += v * v;
    }
    
    stats->mean_value = sum / (float)count;
    stats->rms_value = sqrtf(sum_sq / (float)count);
    
    // Calculate standard deviation
    float var_sum = 0.0f;
    for (int i = 0; i < count; i++) {
        float *value = (float*)((char*)&records[i] + field_offset);
        float diff = *value - stats->mean_value;
        var_sum += diff * diff;
    }
    stats->std_dev = sqrtf(var_sum / (float)count);
    
    return 0;
}

int sim_data_compare(sim_data_record_t *ref, 
                     sim_data_record_t *test,
                     int count,
                     float *tolerances,
                     int max_errors) {
    if (ref == NULL || test == NULL || count <= 0) return -1;
    
    int errors = 0;
    
    // Default tolerances if not provided
    float default_tol[9] = {
        0.0001f,  // time
        0.1f,     // id
        0.1f,     // iq
        0.5f,     // vd
        0.5f,     // vq
        0.01f,    // theta_e
        1.0f,     // omega_e
        1.0f,     // omega_m
        0.01f     // torque
    };
    float *tol = tolerances ? tolerances : default_tol;
    
    for (int i = 0; i < count && errors < max_errors; i++) {
        float *ref_vals = (float*)&ref[i];
        float *test_vals = (float*)&test[i];
        
        for (int j = 0; j < 9; j++) {
            float diff = fabsf(ref_vals[j] - test_vals[j]);
            if (diff > tol[j]) {
                printf("Mismatch at record %d, field %d: ref=%.6f, test=%.6f, diff=%.6f\n",
                       i, j, ref_vals[j], test_vals[j], diff);
                errors++;
                if (errors >= max_errors) break;
            }
        }
    }
    
    return errors;
}

int sim_data_load_reference(const char *filename, 
                            sim_data_record_t *records,
                            int max_records) {
    int result = sim_data_open_read(filename);
    if (result < 0) return -1;
    
    int count = sim_data_read_all(records, max_records);
    sim_data_close_read();
    
    return count;
}
