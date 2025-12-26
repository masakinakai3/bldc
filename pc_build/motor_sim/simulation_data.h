/**
 * @file simulation_data.h
 * @brief Data I/O for simulation results (CSV/binary formats)
 * 
 * Provides functions to record simulation data to files and 
 * read reference data for validation.
 */

#ifndef SIMULATION_DATA_H_
#define SIMULATION_DATA_H_

#include <stdint.h>
#include <stdbool.h>

// Data record structure (one simulation timestep)
typedef struct {
    float time;         // Simulation time [s]
    float id;           // D-axis current [A]
    float iq;           // Q-axis current [A]
    float vd;           // D-axis voltage [V]
    float vq;           // Q-axis voltage [V]
    float theta_e;      // Electrical angle [rad]
    float omega_e;      // Electrical angular velocity [rad/s]
    float omega_m;      // Mechanical angular velocity [rad/s]
    float torque;       // Electromagnetic torque [Nm]
} sim_data_record_t;

// Extended data record (for detailed analysis)
typedef struct {
    sim_data_record_t base;
    float ia, ib, ic;   // Phase currents [A]
    float va, vb, vc;   // Phase voltages [V]
    float p_in;         // Input power [W]
    float p_out;        // Output power [W]
    float p_loss;       // Power loss [W]
    float temp_motor;   // Motor temperature [C]
    float mod_d;        // D-axis modulation index
    float mod_q;        // Q-axis modulation index
} sim_data_record_ext_t;

// File format options
typedef enum {
    SIM_FORMAT_CSV = 0,
    SIM_FORMAT_BINARY,
    SIM_FORMAT_JSON
} sim_data_format_e;

// ==== Write Functions ====

/**
 * @brief Open file for writing simulation data
 * @param filename Path to output file
 * @return 0 on success, -1 on error
 */
int sim_data_open_write(const char *filename);

/**
 * @brief Write a single data record
 */
int sim_data_write_record(sim_data_record_t *rec);

/**
 * @brief Write extended data record
 */
int sim_data_write_record_ext(sim_data_record_ext_t *rec);

/**
 * @brief Close the output file
 */
void sim_data_close(void);

// ==== Read Functions ====

/**
 * @brief Open file for reading simulation data
 * @param filename Path to input file
 * @return Number of records, or -1 on error
 */
int sim_data_open_read(const char *filename);

/**
 * @brief Read a single data record
 * @return 0 on success, 1 on EOF, -1 on error
 */
int sim_data_read_record(sim_data_record_t *rec);

/**
 * @brief Read all records into array
 * @param records Array to store records
 * @param max_records Maximum number to read
 * @return Number of records read
 */
int sim_data_read_all(sim_data_record_t *records, int max_records);

/**
 * @brief Close input file
 */
void sim_data_close_read(void);

// ==== Utility Functions ====

/**
 * @brief Export data to CSV format
 * @param input_file Binary input file
 * @param output_file CSV output file
 * @return 0 on success
 */
int sim_data_export_csv(const char *input_file, const char *output_file);

/**
 * @brief Calculate statistics from data file
 */
typedef struct {
    float min_value;
    float max_value;
    float mean_value;
    float rms_value;
    float std_dev;
} sim_data_stats_t;

int sim_data_calc_stats(sim_data_record_t *records, int count,
                        int field_offset, sim_data_stats_t *stats);

/**
 * @brief Compare two data sets for validation
 * @param ref Reference data
 * @param test Test data
 * @param count Number of records
 * @param tolerances Tolerance for each field (array of 9)
 * @param max_errors Maximum errors to report
 * @return Number of failed comparisons
 */
int sim_data_compare(sim_data_record_t *ref, 
                     sim_data_record_t *test,
                     int count,
                     float *tolerances,
                     int max_errors);

/**
 * @brief Load reference data from MATLAB/Python export
 */
int sim_data_load_reference(const char *filename, 
                            sim_data_record_t *records,
                            int max_records);

#endif // SIMULATION_DATA_H_
