// SPDX-License-Identifier: LGPL-2.1-or-later

#include "gtest/gtest.h"

#include <src/App/InitApplication.h>

#include "PartTestHelpers.h"

std::vector<Part::FilletElement>
_getFilletEdges(std::vector<int> edges = {1, 2}, double startRadius = 0.3, double endRadius = 0.3)
{
    std::vector<Part::FilletElement> filletElements;
    for (auto e : edges) {
        Part::FilletElement fe = {e, startRadius, endRadius};
        filletElements.push_back(fe);
    }
    return filletElements;
}


class FeatureFilletTest: public ::testing::Test, public PartTestHelpers::PartTestHelperClass
{
protected:
    static void SetUpTestSuite()
    {
        tests::initApplication();
    }

    void SetUp() override
    {
        createTestFile();
        _box1obj->Length.setValue(4);
        _box1obj->Width.setValue(5);
        _box1obj->Height.setValue(6);
        _box1obj->Placement.setValue(
            Base::Placement(Base::Vector3d(), Base::Rotation(), Base::Vector3d()));
        _box2obj->Placement.setValue(
            Base::Placement(Base::Vector3d(0, 1, 6), Base::Rotation(), Base::Vector3d()));
        _box2obj->Length.setValue(1);
        _box2obj->Width.setValue(2);
        _box2obj->Height.setValue(3);
        _fused = static_cast<Part::Fuse*>(_doc->addObject("Part::Fuse"));
        _fused->Base.setValue(_box1obj);
        _fused->Tool.setValue(_box2obj);
        _fused->execute();
        _fillet = static_cast<Part::Fillet*>(_doc->addObject("Part::Fillet"));
    }

    void TearDown() override
    {}

    Part::Fuse* _fused;
    Part::Fillet* _fillet;
};

TEST_F(FeatureFilletTest, testInner)
{
    // Arrange
    _fillet->Base.setValue(_fused);
    Part::TopoShape rf = _fused->Shape.getValue();
    std::vector<const char*> get = rf.getElementTypes();
    ASSERT_EQ(get.size(), 3);
    EXPECT_STREQ(get[0], "Face");
    EXPECT_STREQ(get[1], "Edge");
    EXPECT_STREQ(get[2], "Vertex");
    unsigned long sec = rf.countSubElements("Edge");
    EXPECT_EQ(sec, 25);
    _fused->Refine.setValue(true);
    _fused->execute();
    rf = _fused->Shape.getValue();
    sec = rf.countSubElements("Edge");
    EXPECT_EQ(sec, 24);

    _fillet->Edges.setValues(_getFilletEdges());

    // Act
    double volume;

    volume = PartTestHelpers::getVolume(_fused->Shape.getValue());
    EXPECT_DOUBLE_EQ(volume, 126.0);
    volume = PartTestHelpers::getVolume(_fillet->Shape.getValue());
    EXPECT_DOUBLE_EQ(volume, 0.0);
    _fillet->execute();
    volume = PartTestHelpers::getVolume(_fillet->Shape.getValue());
    EXPECT_DOUBLE_EQ(volume, 125.80944686460914);
}

TEST_F(FeatureFilletTest, testOuter)
{
    // Arrange
    _fillet->Base.setValue(_fused);
    _fillet->Edges.setValues(_getFilletEdges({3, 4, 5, 6, 7, 8, 9, 10}));

    // Act
    _fillet->execute();
    Part::TopoShape ts = _fillet->Shape.getValue();
}

// Hmmmm...  FeaturePartCommon with insufficent parameters says MustExecute false,
// but FeatureFillet says MustExecute true ...  Neither of these should really
// happen, though.

TEST_F(FeatureFilletTest, testMustExecute)
{
    // Act
    // short mE = _fillet->mustExecute();
    // // Assert
    // EXPECT_FALSE(mE);
    // _fillet->Base.setValue(_box1obj);
    // // Assert
    // mE = _fillet->mustExecute();
    // EXPECT_FALSE(mE);
    // // Act
    // // _fillet->Edges.setValue(_box2obj);
    // // Assert
    // mE = _fillet->mustExecute();
    // EXPECT_TRUE(mE);
    // _doc->recompute();
    // mE = _fillet->mustExecute();
    // EXPECT_FALSE(mE);
}

TEST_F(FeatureFilletTest, testGetProviderName)
{
    // Act
    _fillet->execute();
    const char* name = _fillet->getViewProviderName();
    // Assert
    EXPECT_STREQ(name, "PartGui::ViewProviderFillet");
}


// TEST_F(FeatureFilletTest, testHistory)
// {
//     // Arrange
//     _fillet->Base.setValue(_box1obj);
//     _fillet->Tool.setValue(_box2obj);

//     // Act and Assert
//     std::vector<Part::ShapeHistory> hist = _fillet->History.getValues();
//     EXPECT_EQ(hist.size(), 0);

//     // This creates the histories classically generated by FreeCAD for comparison
//     using MapList = std::map<int, std::vector<int>>;
//     using List = std::vector<int>;
//     MapList compare1 =
//         {{0, List {0}}, {1, List {5}}, {2, List()}, {3, List {2}}, {4, List {3}}, {5, List {1}}};
//     MapList compare2 =
//         {{0, List {0}}, {1, List {5}}, {2, List {4}}, {3, List()}, {4, List {3}}, {5, List {1}}};

//     _fillet->execute();
//     hist = _fillet->History.getValues();
//     EXPECT_EQ(hist.size(), 2);
//     EXPECT_EQ(hist[0].shapeMap, compare1);
//     EXPECT_EQ(hist[1].shapeMap, compare2);
//     _fillet->Base.setValue(_box2obj);
//     _fillet->Tool.setValue(_box1obj);
//     _fillet->execute();
//     hist = _fillet->History.getValues();
//     // std::cout << testing::PrintToString(hist[0]) << testing::PrintToString(hist[1]);
//     EXPECT_EQ(hist.size(), 2);
//     EXPECT_EQ(hist[1].shapeMap, compare1);
//     EXPECT_EQ(hist[0].shapeMap, compare2);
// }
