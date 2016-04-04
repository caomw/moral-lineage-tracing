#pragma once
#ifndef ANDRES_HDF5_HDF5_HXX
#define ANDRES_HDF5_HDF5_HXX

#include "hdf5.h"

#include <string>
#include <vector>
#include <stdexcept>

namespace andres {
namespace hdf5 {

enum FileAccessMode {READ_ONLY, READ_WRITE};
enum HDF5Version {HDF5_VERSION_DEFAULT, HDF5_VERSION_LATEST};

template<class T> struct HDF5Type { hid_t type() const { throw std::runtime_error("conversion of this type to HDF5 type not implemented."); } };
template<> struct HDF5Type<char> { hid_t type() const { return H5T_NATIVE_CHAR; } };
template<> struct HDF5Type<unsigned char> { hid_t type() const { return H5T_NATIVE_UCHAR; } };
template<> struct HDF5Type<short> { hid_t type() const { return H5T_NATIVE_SHORT; } };
template<> struct HDF5Type<unsigned short> { hid_t type() const { return H5T_NATIVE_USHORT; } };
template<> struct HDF5Type<int> { hid_t type() const { return H5T_NATIVE_INT; } };
template<> struct HDF5Type<unsigned int> { hid_t type() const { return H5T_NATIVE_UINT; } };
template<> struct HDF5Type<long> { hid_t type() const { return H5T_NATIVE_LONG; } };
template<> struct HDF5Type<unsigned long> { hid_t type() const { return H5T_NATIVE_ULONG; } };
template<> struct HDF5Type<long long> { hid_t type() const { return H5T_NATIVE_LLONG; } };
template<> struct HDF5Type<unsigned long long> { hid_t type() const { return H5T_NATIVE_ULLONG; } };
template<> struct HDF5Type<float> { hid_t type() const { return H5T_NATIVE_FLOAT; } };
template<> struct HDF5Type<double> { hid_t type() const { return H5T_NATIVE_DOUBLE; } };
template<> struct HDF5Type<long double> { hid_t type() const { return H5T_NATIVE_LDOUBLE; } };

/// Create an HDF5 file.
///
/// \param filename Name of the file.
/// \param hdf5version HDF5 version tag.
///
/// \returns HDF5 handle
///
/// \sa openFile(), closeFile()
///
inline hid_t
createFile(
    const std::string& filename,
    HDF5Version hdf5version = HDF5_VERSION_DEFAULT
) {
    hid_t version = H5P_DEFAULT;
    if(hdf5version == HDF5_VERSION_LATEST) {
        version = H5Pcreate(H5P_FILE_ACCESS);
        H5Pset_libver_bounds(version, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
    }

    hid_t fileHandle = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, version);
    if(fileHandle < 0) {
        throw std::runtime_error("Could not create HDF5 file: " + filename);
    }

    return fileHandle;
}

/// Open an HDF5 file.
///
/// \param filename Name of the file.
/// \param fileAccessMode File access mode.
/// \param hdf5version HDF5 version tag.
///
/// \returns HDF5 handle
///
/// \sa closeFile(), createFile()
///
inline hid_t
openFile(
    const std::string& filename,
    FileAccessMode fileAccessMode = READ_ONLY,
    HDF5Version hdf5version = HDF5_VERSION_DEFAULT
) {
    hid_t access = H5F_ACC_RDONLY;
    if(fileAccessMode == READ_WRITE) {
        access = H5F_ACC_RDWR;
    }

    hid_t version = H5P_DEFAULT;
    if(hdf5version == HDF5_VERSION_LATEST) {
        version = H5Pcreate(H5P_FILE_ACCESS);
        H5Pset_libver_bounds(version, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
    }

    hid_t fileHandle = H5Fopen(filename.c_str(), access, version);
    if(fileHandle < 0) {
        throw std::runtime_error("Could not open HDF5 file: " + filename);
    }

    return fileHandle;
}

/// Close an HDF5 file
///
/// \param handle Handle to the HDF5 file.
///
/// \sa openFile(), createFile()
///
inline void closeFile(
    const hid_t& handle
) {
    H5Fclose(handle);
}

/// Create an HDF5 group.
///
/// \param parentHandle HDF5 handle on the parent group or file.
/// \param groupName Name of the group.
/// \returns HDF5 handle on the created group
///
/// \sa openGroup(), closeGroup()
///
inline hid_t
createGroup(
    const hid_t& parentHandle,
    const std::string& groupName
) {
    hid_t groupHandle = H5Gcreate(parentHandle, groupName.c_str(),
        H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if(groupHandle < 0) {
        throw std::runtime_error("Could not create HDF5 group.");
    }
    return groupHandle;
}

/// Open an HDF5 group.
///
/// \param parentHandle HDF5 handle on the parent group or file.
/// \param groupName Name of the group.
/// \returns HDF5 handle on the opened group.
///
/// \sa createGroup(), closeGroup()
///
inline hid_t
openGroup(
    const hid_t& parentHandle,
    const std::string& groupName
) {
    hid_t groupHandle = H5Gopen(parentHandle, groupName.c_str(), H5P_DEFAULT);
    if(groupHandle < 0) {
        throw std::runtime_error("Could not open HDF5 group.");
    }
    return groupHandle;
}

/// Close an HDF5 group.
///
/// \param handle HDF5 handle on group to close.
///
/// \sa openGroup(), createGroup()
///
inline void
closeGroup(
    const hid_t& handle
) {
    H5Gclose(handle);
}

/// Save a vector to an HDF5 dataset.
///
template<class T>
inline void
save(
    const hid_t parentHandle,
    const std::string datasetName,
    const std::vector<T>& data
) {
    hsize_t shape[] = {data.size()};
    hid_t dataspace = H5Screate_simple(1, shape, NULL);
    if(dataspace < 0) {
        throw std::runtime_error("could not create HDF5 dataspace.");
    }
    HDF5Type<T> typeMemory;
    hid_t dataset = H5Dcreate(parentHandle, datasetName.c_str(), typeMemory.type(), dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if(dataset < 0) {
        H5Sclose(dataspace);
        throw std::runtime_error("could not create HDF5 dataset.");
    }
    hid_t status = H5Dwrite(dataset, typeMemory.type(), H5S_ALL, H5S_ALL, H5P_DEFAULT, data.data());
    if(status < 0) {
        H5Dclose(dataset);
        H5Sclose(dataspace);
        throw std::runtime_error("could not write to HDF5 dataset.");
    }
}

/// Load a vector from an HDF5 dataset.
///
template<class T>
inline void
load(
    const hid_t parentHandle,
    const std::string datasetName,
    std::vector<T>& data
) {
    // open dataset and get types
    hid_t dataset = H5Dopen(parentHandle, datasetName.c_str(), H5P_DEFAULT);
    if(dataset < 0) {
        throw std::runtime_error("could not open HDF5 dataset.");
    }
    hid_t typeFile = H5Dget_type(dataset);

    // get dimension and shape
    hid_t filespace = H5Dget_space(dataset);
    int dimension = H5Sget_simple_extent_ndims(filespace);
    if(dimension != 1) {
        throw std::runtime_error("HDF5 dataset is not one-dimensional.");
    }
    hsize_t size = 0;
    herr_t status = H5Sget_simple_extent_dims(filespace, &size, NULL);
    if(status < 0) {
        H5Dclose(dataset);
        H5Tclose(typeFile);
        H5Sclose(filespace);
        throw std::runtime_error("could not get shape of HDF5 dataset.");
    }

    // read
    data.resize(size);
    status = H5Dread(dataset, HDF5Type<T>().type(), H5S_ALL, H5S_ALL, H5P_DEFAULT, data.data());

    // close dataset and types
    H5Dclose(dataset);
    H5Tclose(typeFile);
    H5Sclose(filespace);
    if(status < 0) {
        throw std::runtime_error("could not read from HDF5 dataset 'points'.");
    }
}


} // namespace hdf5
} // namespace andres

#endif // #ifndef ANDRES_HDF5_HDF5_HXX
