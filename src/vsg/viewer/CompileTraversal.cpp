/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/viewer/CompileTraversal.h>

#include <vsg/commands/Command.h>
#include <vsg/commands/Commands.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/viewer/CommandGraph.h>
#include <vsg/viewer/RenderGraph.h>
#include <vsg/viewer/View.h>
#include <vsg/viewer/Viewer.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/State.h>

using namespace vsg;

CompileTraversal::CompileTraversal(const CompileTraversal& ct) :
    Inherit(ct)
{
    for (auto& context : ct.contexts)
    {
        contexts.push_back(Context::create(*context));
    }
}

CompileTraversal::CompileTraversal(ref_ptr<Device> device, const ResourceRequirements& resourceRequirements)
{
    overrideMask = vsg::MASK_ALL;
    add(device, resourceRequirements);
}

CompileTraversal::CompileTraversal(Window& window, ref_ptr<ViewportState> viewport, const ResourceRequirements& resourceRequirements)
{
    overrideMask = vsg::MASK_ALL;
    add(window, viewport, resourceRequirements);
}

CompileTraversal::CompileTraversal(const Viewer& viewer, const ResourceRequirements& resourceRequirements)
{
    overrideMask = vsg::MASK_ALL;
    add(viewer, resourceRequirements);
}

CompileTraversal::~CompileTraversal()
{
}

void CompileTraversal::add(ref_ptr<Device> device, const ResourceRequirements& resourceRequirements)
{
    auto queueFamily = device->getPhysicalDevice()->getQueueFamily(VK_QUEUE_GRAPHICS_BIT);
    auto context = Context::create(device, resourceRequirements);
    context->commandPool = CommandPool::create(device, queueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    context->graphicsQueue = device->getQueue(queueFamily);
    contexts.push_back(context);
}

void CompileTraversal::add(Window& window, ref_ptr<ViewportState> viewport, const ResourceRequirements& resourceRequirements)
{
    auto device = window.getOrCreateDevice();
    auto renderPass = window.getOrCreateRenderPass();
    auto queueFamily = device->getPhysicalDevice()->getQueueFamily(VK_QUEUE_GRAPHICS_BIT);
    auto context = Context::create(device, resourceRequirements);
    context->renderPass = renderPass;
    context->commandPool = CommandPool::create(device, queueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    context->graphicsQueue = device->getQueue(queueFamily);

    if (viewport) context->defaultPipelineStates.emplace_back(viewport);
    if (renderPass->maxSamples != VK_SAMPLE_COUNT_1_BIT) context->overridePipelineStates.emplace_back(MultisampleState::create(renderPass->maxSamples));

    contexts.push_back(context);
}

void CompileTraversal::add(Window& window, ref_ptr<View> view, const ResourceRequirements& resourceRequirements)
{
    auto device = window.getOrCreateDevice();
    auto renderPass = window.getOrCreateRenderPass();
    auto queueFamily = device->getPhysicalDevice()->getQueueFamily(VK_QUEUE_GRAPHICS_BIT);
    auto context = Context::create(device, resourceRequirements);
    context->renderPass = renderPass;
    context->commandPool = vsg::CommandPool::create(device, queueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    context->graphicsQueue = device->getQueue(queueFamily);

    if (renderPass->maxSamples != VK_SAMPLE_COUNT_1_BIT) context->overridePipelineStates.emplace_back(vsg::MultisampleState::create(renderPass->maxSamples));

    context->overridePipelineStates.insert(context->overridePipelineStates.end(), view->overridePipelineStates.begin(), view->overridePipelineStates.end());

    auto viewportState = view->camera->viewportState;
    if (viewportState) context->defaultPipelineStates.emplace_back(viewportState);

    context->view = view.get();
    context->viewID = view->viewID;
    context->viewDependentState = view->viewDependentState;

    contexts.push_back(context);
}

void CompileTraversal::add(Framebuffer& framebuffer, ref_ptr<View> view, const ResourceRequirements& resourceRequirements)
{
    auto device = framebuffer.getDevice();
    auto renderPass = framebuffer.getRenderPass();
    auto queueFamily = device->getPhysicalDevice()->getQueueFamily(VK_QUEUE_GRAPHICS_BIT);
    auto context = Context::create(device, resourceRequirements);
    context->renderPass = renderPass;
    context->commandPool = vsg::CommandPool::create(device, queueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    context->graphicsQueue = device->getQueue(queueFamily);

    if (renderPass->maxSamples != VK_SAMPLE_COUNT_1_BIT) context->overridePipelineStates.emplace_back(vsg::MultisampleState::create(renderPass->maxSamples));

    context->overridePipelineStates.insert(context->overridePipelineStates.end(), view->overridePipelineStates.begin(), view->overridePipelineStates.end());

    auto viewportState = view->camera->viewportState;
    if (viewportState) context->defaultPipelineStates.emplace_back(viewportState);

    context->view = view.get();
    context->viewID = view->viewID;
    context->viewDependentState = view->viewDependentState;

    contexts.push_back(context);
}

void CompileTraversal::add(const Viewer& viewer, const ResourceRequirements& resourceRequirements)
{
    struct AddViews : public Visitor
    {
        CompileTraversal* ct = nullptr;
        const ResourceRequirements& resourceRequirements;
        AddViews(CompileTraversal* in_ct, const ResourceRequirements& in_rr) :
            ct(in_ct), resourceRequirements(in_rr){};

        const char* className() const noexcept override { return "vsg::CompileTraversal::AddViews"; }

        std::stack<ref_ptr<Object>> objectStack;

        void apply(Object& object) override
        {
            object.traverse(*this);
        }

        void apply(RenderGraph& rg) override
        {
            if (rg.window)
                objectStack.emplace(rg.window);
            else
                objectStack.emplace(rg.framebuffer);

            rg.traverse(*this);

            objectStack.pop();
        }

        void apply(View& view) override
        {
            if (!objectStack.empty())
            {
                auto obj = objectStack.top();
                if (auto window = obj.cast<Window>())
                    ct->add(*window, ref_ptr<View>(&view), resourceRequirements);
                else if (auto framebuffer = obj.cast<Framebuffer>())
                    ct->add(*framebuffer, ref_ptr<View>(&view), resourceRequirements);
            }
        }
    } addViews(this, resourceRequirements);

    for (auto& task : viewer.recordAndSubmitTasks)
    {
        for (auto& cg : task->commandGraphs)
        {
            cg->accept(addViews);
        }
    }
}

void CompileTraversal::apply(Object& object)
{
    object.traverse(*this);
}

void CompileTraversal::apply(Command& command)
{
    for (auto& context : contexts)
    {
        command.compile(*context);
    }
}

void CompileTraversal::apply(Commands& commands)
{
    for (auto& context : contexts)
    {
        commands.compile(*context);
    }
}

void CompileTraversal::apply(StateGroup& stateGroup)
{
    for (auto& context : contexts)
    {
        stateGroup.compile(*context);
    }
    stateGroup.traverse(*this);
}

void CompileTraversal::apply(Geometry& geometry)
{
    for (auto& context : contexts)
    {
        geometry.compile(*context);
    }
    geometry.traverse(*this);
}

void CompileTraversal::apply(CommandGraph& commandGraph)
{
    auto traverseRenderedSubgraph = [&](vsg::RenderPass* renderpass, VkExtent2D renderArea) {
        uint32_t samples = renderpass->maxSamples;

        for (auto& context : contexts)
        {
            context->renderPass = renderpass;

            // save previous states to be restored after traversal
            auto previousDefaultPipelineStates = context->defaultPipelineStates;
            auto previousOverridePipelineStates = context->overridePipelineStates;

            context->defaultPipelineStates.push_back(ViewportState::create(renderArea));

            if (samples != VK_SAMPLE_COUNT_1_BIT)
            {
                ref_ptr<MultisampleState> defaultMsState = MultisampleState::create(commandGraph.window->framebufferSamples());
                context->overridePipelineStates.push_back(defaultMsState);
            }

            commandGraph.traverse(*this);

            // restore previous values
            context->defaultPipelineStates = previousDefaultPipelineStates;
            context->overridePipelineStates = previousOverridePipelineStates;
        }
    };

    if (commandGraph.framebuffer)
    {
        traverseRenderedSubgraph(commandGraph.framebuffer->getRenderPass(), commandGraph.framebuffer->extent2D());
    }
    else if (commandGraph.window)
    {
        traverseRenderedSubgraph(commandGraph.window->getOrCreateRenderPass(), commandGraph.window->extent2D());
    }
    else
    {
        commandGraph.traverse(*this);
    }
}

void CompileTraversal::apply(RenderGraph& renderGraph)
{
    for (auto& context : contexts)
    {
        context->renderPass = renderGraph.getRenderPass();

        // save previous states to be restored after traversal
        auto previousDefaultPipelineStates = context->defaultPipelineStates;
        auto previousOverridePipelineStates = context->overridePipelineStates;

        if (renderGraph.window)
        {
            context->defaultPipelineStates.push_back(ViewportState::create(renderGraph.window->extent2D()));
        }
        else if (renderGraph.framebuffer)
        {
            VkExtent2D extent{renderGraph.framebuffer->width(), renderGraph.framebuffer->height()};
            context->defaultPipelineStates.push_back(ViewportState::create(extent));
        }

        if (context->renderPass && context->renderPass->maxSamples != VK_SAMPLE_COUNT_1_BIT)
        {
            ref_ptr<MultisampleState> defaultMsState = MultisampleState::create(context->renderPass->maxSamples);
            context->overridePipelineStates.push_back(defaultMsState);
        }

        renderGraph.traverse(*this);

        // restore previous values
        context->defaultPipelineStates = previousDefaultPipelineStates;
        context->overridePipelineStates = previousOverridePipelineStates;
    }
}

void CompileTraversal::apply(View& view)
{
    for (auto& context : contexts)
    {
        context->viewID = view.viewID;
        context->viewDependentState = view.viewDependentState.get();
        if (view.viewDependentState) view.viewDependentState->compile(*context);

        // save previous pipeline states
        auto previousOverridePipelineStates = context->overridePipelineStates;
        auto previousDefaultPipelineStates = context->defaultPipelineStates;

        // assign view specific pipeline states
        context->overridePipelineStates.insert(context->overridePipelineStates.end(), view.overridePipelineStates.begin(), view.overridePipelineStates.end());
        if (view.camera && view.camera->viewportState) context->defaultPipelineStates.emplace_back(view.camera->viewportState);

        view.traverse(*this);

        // restore previous pipeline states
        context->defaultPipelineStates = previousDefaultPipelineStates;
        context->overridePipelineStates = previousOverridePipelineStates;
    }
}

bool CompileTraversal::record()
{
    bool recorded = false;
    for (auto& context : contexts)
    {
        if (context->record()) recorded = true;
    }
    return recorded;
}

void CompileTraversal::waitForCompletion()
{
    for (auto& context : contexts)
    {
        context->waitForCompletion();
    }
}
