/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-07-01 13:38:02
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-01 13:38:03
 */
#pragma once

#include <iostream>
#include <Eigen/Eigen>

typedef Eigen::AlignedBox3f BoundingBox3f;

// 扩充一些额外的类型定义, uint8_t , uint16_t , uint32_t 
namespace Eigen {
	/** \defgroup matrixtypedefs Global matrix typedefs
	 *
	 * \ingroup Core_Module
	 *
	 * Eigen defines several typedef shortcuts for most common matrix and vector types.
	 *
	 * The general patterns are the following:
	 *
	 * \c MatrixSizeType where \c Size can be \c 2,\c 3,\c 4 for fixed size square matrices or \c X for dynamic size,
	 * and where \c Type can be \c i for integer, \c f for float, \c d for double, \c cf for complex float, \c cd
	 * for complex double.
	 *
	 * For example, \c Matrix3d is a fixed-size 3x3 matrix type of doubles, and \c MatrixXf is a dynamic-size matrix of
	 * floats.
	 *
	 * There are also \c VectorSizeType and \c RowVectorSizeType which are self-explanatory. For example, \c Vector4cf is
	 * a fixed-size vector of 4 complex floats.
	 *
	 * \sa class Matrix
	 */

#define EIGEN_MAKE_TYPEDEFS(Type, TypeSuffix, Size, SizeSuffix)                                                        \
    /** \ingroup matrixtypedefs */                                                                                     \
    typedef Matrix<Type, Size, Size> Matrix##SizeSuffix##TypeSuffix;                                                   \
    /** \ingroup matrixtypedefs */                                                                                     \
    typedef Matrix<Type, Size, 1> Vector##SizeSuffix##TypeSuffix;                                                      \
    /** \ingroup matrixtypedefs */                                                                                     \
    typedef Matrix<Type, 1, Size> RowVector##SizeSuffix##TypeSuffix;

#define EIGEN_MAKE_FIXED_TYPEDEFS(Type, TypeSuffix, Size)                                                              \
    /** \ingroup matrixtypedefs */                                                                                     \
    typedef Matrix<Type, Size, Dynamic> Matrix##Size##X##TypeSuffix;                                                   \
    /** \ingroup matrixtypedefs */                                                                                     \
    typedef Matrix<Type, Dynamic, Size> Matrix##X##Size##TypeSuffix;

#define EIGEN_MAKE_TYPEDEFS_ALL_SIZES(Type, TypeSuffix)                                                                \
    EIGEN_MAKE_TYPEDEFS(Type, TypeSuffix, 2, 2)                                                                        \
    EIGEN_MAKE_TYPEDEFS(Type, TypeSuffix, 3, 3)                                                                        \
    EIGEN_MAKE_TYPEDEFS(Type, TypeSuffix, 4, 4)                                                                        \
    EIGEN_MAKE_TYPEDEFS(Type, TypeSuffix, Dynamic, X)                                                                  \
    EIGEN_MAKE_FIXED_TYPEDEFS(Type, TypeSuffix, 2)                                                                     \
    EIGEN_MAKE_FIXED_TYPEDEFS(Type, TypeSuffix, 3)                                                                     \
    EIGEN_MAKE_FIXED_TYPEDEFS(Type, TypeSuffix, 4)

	EIGEN_MAKE_TYPEDEFS_ALL_SIZES(uint8_t, i_8)
	EIGEN_MAKE_TYPEDEFS_ALL_SIZES(uint16_t, i_16)
	EIGEN_MAKE_TYPEDEFS_ALL_SIZES(uint32_t, i_32)

#undef EIGEN_MAKE_TYPEDEFS_ALL_SIZES
#undef EIGEN_MAKE_TYPEDEFS
#undef EIGEN_MAKE_FIXED_TYPEDEFS
} // namespace Eigen
