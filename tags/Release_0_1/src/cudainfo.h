/*!
	\file cudainfo.h
	\brief CUDA information data and function definition.
	\author AG
*/

#ifndef CZ_CUDAINFO_H
#define CZ_CUDAINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/*!
	\brief Information about CUDA-device core.
*/
struct CZDeviceInfoCore {
	int		regsPerBlock;		/*!< Total number of registers available per block. */
	int		SIMDWidth;		/*!< Warp size. */
	int		maxThreadsPerBlock;	/*!< Maximum number of threads per block. */
	int		maxThreadsDim[3];	/*!< Maximum sizes of each dimension of a block. */
	int		maxGridSize[3];		/*!< Maximum sizes of each dimension of a grid. */
	int		clockRate;		/*!< Clock frequency in kilohertz. */
};

/*!
	\brief Information about CUDA-device memory.
*/
struct CZDeviceInfoMem {
	int		totalGlobal;		/*!< Total amount of global memory available on the device in bytes. */
	int		sharedPerBlock;		/*!< Total amount of shared memory available per block in bytes. */
	int		maxPitch;		/*!< Maximum pitch allowed by the memory copy functions that involve memory region allocated through cudaMallocPitch()/cuMemAllocPitch() */
	int		totalConst;		/*!< Total amount of constant memory available on the device in bytes. */
	int		textureAlignment;	/*!< Texture base addresses that are aligned to textureAlignment bytes do not need an offset applied to texture fetches. */
	int		gpuOverlap;		/*!< 1 if the device can concurrently copy memory between host and device while executing a kernel, or 0 if not. */
};

/*!
	\brief Information about CUDA-device.
*/
struct CZDeviceInfo {
	int		num;			/*!< Device index */
	char		deviceName[256];	/*!< ASCII string identifying the device. */
	int		major;			/*!< Major revision numbers defining the device's compute capability. */
	int		minor;			/*!< Minor revision numbers defining the device's compute capability. */
	struct CZDeviceInfoCore	core;
	struct CZDeviceInfoMem	mem;
};

bool cudaCheck(void);
int cudaDeviceFound(void);
int cudaReadDeviceInfo(struct CZDeviceInfo *info, int num);

#ifdef __cplusplus
}
#endif

#endif//CZ_CUDAINFO_H