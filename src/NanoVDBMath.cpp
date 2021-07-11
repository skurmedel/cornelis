#include <cornelis/NanoVDBMath.hpp>

template class nanovdb::Vec3<float>;
template class nanovdb::Vec3<double>;
template class nanovdb::Vec4<float>;
template class nanovdb::Vec4<double>;

template struct nanovdb::BBox<nanovdb::Vec3<float>>;
template struct nanovdb::BBox<nanovdb::Vec3<double>>;

template struct nanovdb::Ray<float>;
