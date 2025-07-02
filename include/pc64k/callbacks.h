#pragma once
#include <stddef.h>
#include <stdint.h>

// Returns disk size
typedef size_t(*PC64KDiskSizeGetter)();
// Reads data from disk.
typedef void(*PC64KDiskReader)(size_t pos, uint8_t* data, uint8_t len);
// Writes data to disk.
typedef void(*PC64KDiskWriter)(size_t pos, uint8_t* data, uint8_t len);

// Gets microseconds since an arbitrary point in time
typedef uint64_t(*PC64KMicrosGetter)();