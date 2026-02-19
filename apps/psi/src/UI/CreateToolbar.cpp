#include "CreateToolbar.h"
#include "UISettings.h"
#include "layers/PsiWorldLayer.h"
#include "layers/PsiNodeEditorLayer.h"
#include "imgui.h"
#include "IconsLucide.h"
#include "utils/TextIcons.h"
#include "Layers/DefaultGameWorld/Mesh.h"

#include <generator/BoxMesh.hpp>
#include <generator/TriangleMesh.hpp>
#include <generator/SphereMesh.hpp>
#include <generator/CylinderMesh.hpp>
#include <generator/TorusMesh.hpp>
#include <generator/ConeMesh.hpp>
#include <generator/DiskMesh.hpp>

#include "nodes/GraphNodes.h"
#include "nodes/NodeSystem.h"
#include "nodes/ValueNodes.h"
#include "nodes/MathNodes.h"
#include "nodes/VectorNodes.h"
#include "nodes/ObjectNodes.h"
#include "PsiColors.h"

namespace {
    template<typename GeneratorMesh>
    Mesh convertFromGenerator(GeneratorMesh&& genMesh)
    {
        Mesh mesh;

        auto vertexGen = genMesh.vertices();
        while (!vertexGen.done()) {
            auto v = vertexGen.generate();
            mesh.vertices.push_back({
                glm::vec3(static_cast<float>(v.position[0]),
                          static_cast<float>(v.position[1]),
                          static_cast<float>(v.position[2])),
                glm::vec3(static_cast<float>(v.normal[0]),
                          static_cast<float>(v.normal[1]),
                          static_cast<float>(v.normal[2])),
                glm::vec2(static_cast<float>(v.texCoord[0]),
                          static_cast<float>(v.texCoord[1])),
                glm::vec4(1.0f)
            });
            vertexGen.next();
        }

        auto triangleGen = genMesh.triangles();
        while (!triangleGen.done()) {
            auto tri = triangleGen.generate();
            mesh.indices.push_back(static_cast<uint32_t>(tri.vertices[0]));
            mesh.indices.push_back(static_cast<uint32_t>(tri.vertices[1]));
            mesh.indices.push_back(static_cast<uint32_t>(tri.vertices[2]));
            triangleGen.next();
        }

        return mesh;
    }
}

CreateToolbar::CreateToolbar(PsiWorldLayer* worldLayer, PsiNodeEditorLayer* nodeEditorLayer)
    : m_WorldLayer(worldLayer)
    , m_NodeEditorLayer(nodeEditorLayer)
{
}

void CreateToolbar::Render()
{
    const UISettings& s = g_UISettings;

    // Position above the GizmoToolbar in the top-left corner
    ImGui::SetNextWindowPos(ImVec2(s.gizmoLeftMargin, s.createTopMargin), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);

    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration          |
        ImGuiWindowFlags_NoMove                |
        ImGuiWindowFlags_NoSavedSettings       |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(s.gizmoFramePadX, s.gizmoFramePadY));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,  ImVec2(s.gizmoItemSpacingX, s.gizmoItemSpacingY));

    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, s.gizmoHoverAlpha));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(1.0f, 1.0f, 1.0f, s.gizmoActiveAlpha));

    ImGui::Begin("##create_toolbar", nullptr, flags);
    ImGui::SetWindowFontScale(s.gizmoFontScale);

    const glm::vec3& g = psi::Colors::AccentBlue;
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(g.r, g.g, g.b, 1.0f));
    if (UIUtils::IconButton(ICON_LC_PLUS))
        ImGui::OpenPopup("##create_popup");
    ImGui::PopStyleColor();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,  ImVec2(12.0f, 10.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,    ImVec2(16.0f, 10.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);

    if (ImGui::BeginPopup("##create_popup"))
    {
        ImGui::SetWindowFontScale(s.popupFontScale);

        if (UIUtils::IconBeginMenu(ICON_LC_SHAPES, "Meshes"))
        {
            RenderMeshMenu();
            ImGui::EndMenu();
        }

        if (m_NodeEditorLayer && UIUtils::IconBeginMenu(ICON_LC_WORKFLOW, "Nodes"))
        {
            RenderNodeMenu();
            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(3);

    ImGui::End();

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
}

void CreateToolbar::RenderMeshMenu()
{
    if (!m_WorldLayer)
        return;

    ImGui::SetWindowFontScale(g_UISettings.popupFontScale);

    if (UIUtils::IconMenuItem(ICON_LC_BOX, "Cube"))
    {
        auto mesh = MeshTable::unitCube();
        m_WorldLayer->addMeshPrimitive("Cube", std::move(mesh),
            glm::vec3(0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    }

    if (UIUtils::IconMenuItem(ICON_LC_TRIANGLE, "Triangle"))
    {
        auto mesh = MeshTable::unitTriangle();
        m_WorldLayer->addMeshPrimitive("Triangle", std::move(mesh),
            glm::vec3(0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
    }

    if (UIUtils::IconMenuItem(ICON_LC_CIRCLE, "Sphere"))
    {
        auto mesh = convertFromGenerator(generator::SphereMesh(1.0, 32, 16));
        m_WorldLayer->addMeshPrimitive("Sphere", std::move(mesh),
            glm::vec3(0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
    }

    if (UIUtils::IconMenuItem(ICON_LC_CYLINDER, "Cylinder"))
    {
        auto mesh = convertFromGenerator(generator::CylinderMesh(1.0, 1.0, 2.0, 32, 8));
        m_WorldLayer->addMeshPrimitive("Cylinder", std::move(mesh),
            glm::vec3(0.0f), glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
    }

    if (UIUtils::IconMenuItem(ICON_LC_TORUS, "Torus"))
    {
        auto mesh = convertFromGenerator(generator::TorusMesh(0.25, 1.0, 32, 16));
        m_WorldLayer->addMeshPrimitive("Torus", std::move(mesh),
            glm::vec3(0.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
    }

    if (UIUtils::IconMenuItem(ICON_LC_CONE, "Cone"))
    {
        auto mesh = convertFromGenerator(generator::ConeMesh(1.0, 2.0, 32, 8));
        m_WorldLayer->addMeshPrimitive("Cone", std::move(mesh),
            glm::vec3(0.0f), glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));
    }

    if (UIUtils::IconMenuItem(ICON_LC_CIRCLE_DOT, "Circle"))
    {
        auto mesh = convertFromGenerator(generator::DiskMesh(1.0, 0.0, 32, 4));
        m_WorldLayer->addMeshPrimitive("Circle", std::move(mesh),
            glm::vec3(0.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }
}

void CreateToolbar::RenderNodeMenu()
{
    NodeGraph& graph = m_NodeEditorLayer->getNodeGraph();

    ImGui::SetWindowFontScale(g_UISettings.popupFontScale);

    if (UIUtils::IconBeginMenu(ICON_LC_CHART_LINE, "Graph Nodes"))
    {
        ImGui::SetWindowFontScale(g_UISettings.popupFontScale);
        if (UIUtils::IconMenuItem(ICON_LC_NETWORK, "Graph Node"))
            graph.createNode<LineGraphNode>();
        ImGui::EndMenu();
    }

    if (UIUtils::IconBeginMenu(ICON_LC_VARIABLE, "Value Nodes"))
    {
        ImGui::SetWindowFontScale(g_UISettings.popupFontScale);
        if (UIUtils::IconMenuItem(ICON_LC_VARIABLE,     "Float Constant"))
            graph.createNode<FloatConstantNode>(0.0f);
        if (UIUtils::IconMenuItem(ICON_LC_VARIABLE,     "Int Constant"))
            graph.createNode<IntConstantNode>(0);
        if (UIUtils::IconMenuItem(ICON_LC_VECTOR_SQUARE,"Vec3 Constant"))
            graph.createNode<Vec3ConstantNode>(glm::vec3(0.0f));
        if (UIUtils::IconMenuItem(ICON_LC_WAVES,        "Time"))
            graph.createNode<TimeNode>();
        if (UIUtils::IconMenuItem(ICON_LC_ATOM,         "Physics Constant"))
            graph.createNode<PhysicsConstantNode>();
        ImGui::EndMenu();
    }

    if (UIUtils::IconBeginMenu(ICON_LC_SIGMA, "Math Nodes"))
    {
        ImGui::SetWindowFontScale(g_UISettings.popupFontScale);
        if (UIUtils::IconMenuItem(ICON_LC_PLUS,            "Add"))       graph.createNode<AddNode>();
        if (UIUtils::IconMenuItem(ICON_LC_MINUS,           "Subtract"))  graph.createNode<SubtractNode>();
        if (UIUtils::IconMenuItem(ICON_LC_ASTERISK,        "Multiply"))  graph.createNode<MultiplyNode>();
        if (UIUtils::IconMenuItem(ICON_LC_DIVIDE,          "Divide"))    graph.createNode<DivideNode>();
        if (UIUtils::IconMenuItem(ICON_LC_WAVES,           "Sin"))       graph.createNode<SinNode>();
        if (UIUtils::IconMenuItem(ICON_LC_WAVES,           "Cos"))       graph.createNode<CosNode>();
        if (UIUtils::IconMenuItem(ICON_LC_WAVES,           "Tan"))       graph.createNode<TanNode>();
        if (UIUtils::IconMenuItem(ICON_LC_WAVES,           "Arcsin"))    graph.createNode<ArcsinNode>();
        if (UIUtils::IconMenuItem(ICON_LC_WAVES,           "Arccos"))    graph.createNode<ArccosNode>();
        if (UIUtils::IconMenuItem(ICON_LC_WAVES,           "Arctan"))    graph.createNode<ArctanNode>();
        if (UIUtils::IconMenuItem(ICON_LC_SQUARE_FUNCTION, "Pow"))       graph.createNode<PowNode>();
        if (UIUtils::IconMenuItem(ICON_LC_RADICAL,         "Root"))      graph.createNode<RootNode>();
        if (UIUtils::IconMenuItem(ICON_LC_PI,              "PI"))        graph.createNode<PINode>();
        ImGui::EndMenu();
    }

    if (UIUtils::IconBeginMenu(ICON_LC_VECTOR_SQUARE, "Vector Nodes"))
    {
        ImGui::SetWindowFontScale(g_UISettings.popupFontScale);
        if (UIUtils::IconMenuItem(ICON_LC_LAYERS,   "Combine Vec3"))   graph.createNode<CombineVec3Node>();
        if (UIUtils::IconMenuItem(ICON_LC_LAYERS_2, "Separate Vec3"))  graph.createNode<SeparateVec3Node>();
        if (UIUtils::IconMenuItem(ICON_LC_DOT,      "Dot Product"))    graph.createNode<DotProductNode>();
        if (UIUtils::IconMenuItem(ICON_LC_X,        "Cross Product"))  graph.createNode<CrossProductNode>();
        if (UIUtils::IconMenuItem(ICON_LC_RULER,    "Length"))         graph.createNode<LengthNode>();
        ImGui::EndMenu();
    }

    if (UIUtils::IconBeginMenu(ICON_LC_LAYERS, "Object Nodes"))
    {
        ImGui::SetWindowFontScale(g_UISettings.popupFontScale);
        if (UIUtils::IconMenuItem(ICON_LC_MOVE_3D, "Transform"))      graph.createNode<TransformNode>();
        if (UIUtils::IconMenuItem(ICON_LC_SPLINE,  "Line Renderer"))  graph.createNode<LineRendererNode>();
        ImGui::EndMenu();
    }
}
