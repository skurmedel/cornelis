#pragma once
// This is mainly a way to speed up compilation a bit by using explicit & extern templates.

#include <nanovdb/NanoVDB.h>
#include <nanovdb/util/Ray.h>

extern template class nanovdb::Vec3<float>;
extern template class nanovdb::Vec3<double>;
extern template class nanovdb::Vec4<float>;
extern template class nanovdb::Vec4<double>;

extern template struct nanovdb::BBox<nanovdb::Vec3<float>>;
extern template struct nanovdb::BBox<nanovdb::Vec3<double>>;

extern template struct nanovdb::Ray<float>;