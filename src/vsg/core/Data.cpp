/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Allocator.h>
#include <vsg/core/Data.h>
#include <vsg/io/Input.h>
#include <vsg/io/Options.h>
#include <vsg/io/Output.h>

using namespace vsg;

void* Data::operator new(std::size_t count)
{
    return vsg::allocate(count, vsg::ALLOCATOR_AFFINITY_DATA);
}

void Data::operator delete(void* ptr)
{
    vsg::deallocate(ptr);
}

void Data::read(Input& input)
{
    Object::read(input);

    uint32_t format = 0;

    if (input.version_greater_equal(0, 6, 1))
    {
        input.read("properties", format, properties.stride, properties.maxNumMipmaps, properties.blockWidth, properties.blockHeight, properties.blockDepth, properties.origin, properties.imageViewType, properties.dataVariance);
    }
    else if (input.version_greater_equal(0, 5, 7))
    {
        input.read("Layout", format, properties.stride, properties.maxNumMipmaps, properties.blockWidth, properties.blockHeight, properties.blockDepth, properties.origin, properties.imageViewType, properties.dataVariance);
    }
    else
    {
        input.read("Layout", format, properties.stride, properties.maxNumMipmaps, properties.blockWidth, properties.blockHeight, properties.blockDepth, properties.origin, properties.imageViewType);
        properties.dataVariance = STATIC_DATA;
    }

    properties.format = VkFormat(format);
}

void Data::write(Output& output) const
{
    Object::write(output);

    uint32_t format = properties.format;
    if (output.version_greater_equal(0, 6, 1))
    {
        output.write("properties", format, properties.stride, properties.maxNumMipmaps, properties.blockWidth, properties.blockHeight, properties.blockDepth, properties.origin, properties.imageViewType, properties.dataVariance);
    }
    else if (output.version_greater_equal(0, 5, 7))
    {
        output.write("Layout", format, properties.stride, properties.maxNumMipmaps, properties.blockWidth, properties.blockHeight, properties.blockDepth, properties.origin, properties.imageViewType, properties.dataVariance);
    }
    else
    {
        output.write("Layout", format, properties.stride, properties.maxNumMipmaps, properties.blockWidth, properties.blockHeight, properties.blockDepth, properties.origin, properties.imageViewType);
    }
}

Data::MipmapOffsets Data::computeMipmapOffsets() const
{
    uint32_t numMipmaps = properties.maxNumMipmaps;

    MipmapOffsets offsets;
    if (numMipmaps == 0) return offsets;

    std::size_t w = width();
    std::size_t h = height();
    std::size_t d = depth();

    std::size_t lastPosition = 0;
    offsets.push_back(lastPosition);
    while (numMipmaps > 1 && (w > 1 || h > 1 || d > 1))
    {
        lastPosition += (w * h * d);
        offsets.push_back(lastPosition);

        --numMipmaps;
        if (w > 1) w /= 2;
        if (h > 1) h /= 2;
        if (d > 1) d /= 2;
    }

    return offsets;
}

std::size_t Data::computeValueCountIncludingMipmaps(std::size_t w, std::size_t h, std::size_t d, uint32_t numMipmaps)
{
    if (numMipmaps <= 1) return w * h * d;

    std::size_t lastPosition = (w * h * d);
    while (numMipmaps > 1 && (w > 1 || h > 1 || d > 1))
    {
        --numMipmaps;

        if (w > 1) w /= 2;
        if (h > 1) h /= 2;
        if (d > 1) d /= 2;

        lastPosition += (w * h * d);
    }

    return lastPosition;
}
