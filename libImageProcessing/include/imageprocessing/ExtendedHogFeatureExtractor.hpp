/*
 * ExtendedHogFeatureExtractor.hpp
 *
 *  Created on: 28.03.2014
 *      Author: poschmann
 */

#ifndef EXTENDEDHOGFEATUREEXTRACTOR_HPP_
#define EXTENDEDHOGFEATUREEXTRACTOR_HPP_

#include "imageprocessing/FeatureExtractor.hpp"
#include <vector>

namespace imageprocessing {

class ImagePyramid;
class ImageFilter;
class GradientFilter;
class GradientBinningFilter;
class ExtendedHogFilter;
class CompleteExtendedHogFilter;

/**
 * Feature extractor that computes extended HOG features on the extracted patches. The features are concatenated
 * cell descriptors that depend on the pixels within the cell and on the features of the adjacent cells. Because
 * of that, the extracted patches will also capture their surroundings (patch is extended to include the cells
 * next to it). The additional cells are removed from the final feature vector. If the additional cells are
 * (partially) outside the image (of the pyramid layer), then imaginary pixel values will be generated by
 * reflecting the image at the border.
 */
class ExtendedHogFeatureExtractor : public FeatureExtractor {
public:

	/**
	 * Constructs a new extended HOG feature extractor on top of the given image pyramid. The image pyramid layers
	 * are expected to have grayscale images.
	 *
	 * @param[in] pyramid The image pyramid.
	 * @param[in] ehogFilter The extended HOG filter.
	 * @param[in] cols The column count of cells.
	 * @param[in] rows The row count of cells.
	 */
	ExtendedHogFeatureExtractor(std::shared_ptr<ImagePyramid> pyramid,
			std::shared_ptr<CompleteExtendedHogFilter> ehogFilter, int cols, int rows);

	/**
	 * Constructs a new extended HOG feature extractor on top of the given image pyramid. The images of the pyramid
	 * layers are expected to contain bin informations (e.g. GradientBinningFilter is applied).
	 *
	 * @param[in] pyramid The image pyramid.
	 * @param[in] ehogFilter The extended HOG filter.
	 * @param[in] cols The column count of cells.
	 * @param[in] rows The row count of cells.
	 */
	ExtendedHogFeatureExtractor(std::shared_ptr<ImagePyramid> pyramid, std::shared_ptr<ExtendedHogFilter> ehogFilter, int cols, int rows);

	/**
	 * Constructs a new extended HOG feature extractor that creates an image pyramid from the given patch widths.
	 * A filter will convert the images to grayscale before scaling them down.
	 *
	 * @param[in] ehogFilter The extended HOG filter.
	 * @param[in] cols The column count of cells.
	 * @param[in] rows The row count of cells.
	 * @param[in] minWidth The width of the smallest patches that will be extracted.
	 * @param[in] maxWidth The width of the biggest patches that will be extracted.
	 * @param[in] octaveLayerCount The number of layers per octave.
	 */
	ExtendedHogFeatureExtractor(std::shared_ptr<CompleteExtendedHogFilter> ehogFilter,
			int cols, int rows, int minWidth, int maxWidth, int octaveLayerCount = 5);

	/**
	 * Constructs a new extended HOG feature extractor that creates an image pyramid from the given patch widths.
	 * A filter will convert the images to grayscale before scaling them down. The gradient and binning filter will
	 * be applied to the scaled down images of the pyramid layers.
	 *
	 * @param[in] gradientFilter The gradient gradient filter.
	 * @param[in] binningFilter The gradient binning filter.
	 * @param[in] ehogFilter The extended HOG filter.
	 * @param[in] cols The column count of cells.
	 * @param[in] rows The row count of cells.
	 * @param[in] minWidth The width of the smallest patches that will be extracted.
	 * @param[in] maxWidth The width of the biggest patches that will be extracted.
	 * @param[in] octaveLayerCount The number of layers per octave.
	 */
	ExtendedHogFeatureExtractor(std::shared_ptr<GradientFilter> gradientFilter,
			std::shared_ptr<GradientBinningFilter> binningFilter, std::shared_ptr<ExtendedHogFilter> ehogFilter,
			int cols, int rows, int minWidth, int maxWidth, int octaveLayerCount = 5);

	/**
	 * Copy constructor.
	 *
	 * @param[in] other The extractor to make a copy of.
	 */
	ExtendedHogFeatureExtractor(const ExtendedHogFeatureExtractor& other);

	using FeatureExtractor::update;

	void update(std::shared_ptr<VersionedImage> image);

	std::shared_ptr<Patch> extract(int x, int y, int width, int height) const;

	/**
	 * @return The image pyramid.
	 */
	std::shared_ptr<ImagePyramid> getPyramid();

	/**
	 * @return The image pyramid.
	 */
	const std::shared_ptr<ImagePyramid> getPyramid() const;

	/**
	 * @return The width of the image data of the extracted patches (before applying the extended HOG filter).
	 */
	int getPatchWidth() const;

	/**
	 * @return The height of the image data of the extracted patches (before applying the extended HOG filter).
	 */
	int getPatchHeight() const;

private:

	/**
	 * Creates a new image pyramid whose min and max scale factors are chosen to enable the extraction of patches
	 * of certain widths.
	 *
	 * @param[in] width The width of the extracted patch data.
	 * @param[in] minWidth The width of the smallest patches that will be extracted.
	 * @param[in] maxWidth The width of the biggest patches that will be extracted.
	 * @param[in] octaveLayerCount The number of layers per octave.
	 * @return A newly created image pyramid.
	 */
	static std::shared_ptr<ImagePyramid> createPyramid(int width, int minWidth, int maxWidth, int octaveLayerCount);

	/**
	 * Creates the look-up table for the image indices that are used to retrieve the patch data.
	 *
	 * @param[in] imageSize The size of the image (width or height).
	 * @param[in] patchStart The first patch index inside the image (x or y).
	 * @param[in] patchSize The size of the patch (width or height).
	 * @return A vector mapping the patch indices to image indices.
	 */
	std::vector<int> createIndexLut(int imageSize, int patchStart, int patchSize) const;

	/**
	 * Creates the patch data by copying values from the image.
	 *
	 * @param[in] image The image to take the values from.
	 * @param[in] rowIndices The mappings from patch row indices to image row indices.
	 * @param[in] colIndices The mappings from patch column indices to image column indices.
	 * @return The patch data.
	 */
	template<class T>
	cv::Mat createPatchData(const cv::Mat& image, std::vector<int>& rowIndices, std::vector<int>& colIndices) const {
		cv::Mat patch(rowIndices.size(), colIndices.size(), image.type());
		for (size_t patchY = 0; patchY < rowIndices.size(); ++patchY) {
			T* patchRow = patch.ptr<T>(patchY);
			const T* imageRow = image.ptr<T>(rowIndices[patchY]);
			for (size_t patchX = 0; patchX < colIndices.size(); ++patchX)
				patchRow[patchX] = imageRow[colIndices[patchX]];
		}
		return patch;
	}

	std::shared_ptr<ImagePyramid> pyramid; ///< Image pyramid.
	std::shared_ptr<ImageFilter> ehogFilter; ///< Extended HOG filter that is applied to the patches.
	int patchWidth;  ///< Width of the image data of the extracted patches.
	int patchHeight; ///< Height of the image data of the extracted patches.
	int cellSize; ///< Width and height of the cells.
	double widthFactor;  ///< Scale factor for increasing the patch width before extraction to capture surrounding cells.
	double heightFactor; ///< Scale factor for increasing the patch height before extraction to capture surrounding cells.
};

} /* namespace imageprocessing */
#endif /* EXTENDEDHOGFEATUREEXTRACTOR_HPP_ */