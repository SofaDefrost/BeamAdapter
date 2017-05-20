/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, version 1.0 beta 4      *
*                (c) 2006-2009 MGH, INRIA, USTL, UJF, CNRS                    *
*                                                                             *
* This library is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This library is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this library; if not, write to the Free Software Foundation,     *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.          *
*******************************************************************************
*                               SOFA :: Modules                               *
*                                                                             *
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
//
// C++ Implementation : AdaptiveBeamController
//
// Description:
//
//
// Author: Christian Duriez, INRIA
//
// Copyright: See COPYING file that comes with this distribution
//
//
//

#ifndef SOFA_COMPONENT_CONTROLLER_ADAPTIVEBEAMCONTROLLER_H
#define SOFA_COMPONENT_CONTROLLER_ADAPTIVEBEAMCONTROLLER_H

//////////////////////// Inclusion of headers...from wider to narrower/closer //////////////////////
#include <SofaUserInteraction/MechanicalStateController.h>
#include <SofaBoundaryCondition/FixedConstraint.h>
#include <SofaMeshCollision/PointModel.h>
#include <SofaMeshCollision/LineModel.h>

#include "BeamInterpolation.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Forward declarations, see https://en.wikipedia.org/wiki/Forward_declaration
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace sofa {
    namespace component {
        namespace topology {
            template <class T>
            class EdgeSetGeometryAlgorithms;
            class EdgeSetTopologyModifier;
        }
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Declarations
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace sofa
{
namespace component
{
namespace controller
{

/////////////////////////////////// private namespace pattern //////////////////////////////////////
/// To avoid the lacking of names imported with with 'using' in the other's component namespace
/// you should use a private namespace and "export" only this one in the public namespace.
/// This is done at the end of this file, have a look if you are not used to this pattern.
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace _adaptivebeamcontroller_
{

using sofa::component::projectiveconstraintset::FixedConstraint ;
using sofa::component::topology::EdgeSetTopologyModifier ;
using sofa::component::topology::EdgeSetGeometryAlgorithms ;
using sofa::component::fem::BeamInterpolation ;
using sofa::component::collision::PointActiver ;
using sofa::component::collision::LineActiver ;
using sofa::core::topology::BaseMeshTopology ;
using sofa::defaulttype::SolidTypes ;
using sofa::defaulttype::Vec ;
using sofa::helper::vector ;

// TODO(dmarchal 2017-05-17) to eulalie & christian is the following still valid ?
/**
 * \class AdaptiveBeamController
 * @brief AdaptiveBeamController Mouse & Keyboard controller for EdgeSetTopology
 *
 * This component provides an interaction technique based on Mouse & Keyboard that allow user to
 * control on an EdgeSet Topology.
 */
template<class DataTypes>
class AdaptiveBeamController : public MechanicalStateController<DataTypes>,
                               public PointActiver,
                               public LineActiver
{
public:
    SOFA_CLASS(SOFA_TEMPLATE(AdaptiveBeamController,DataTypes),
               SOFA_TEMPLATE(MechanicalStateController,DataTypes));

    typedef typename DataTypes::VecCoord VecCoord;
    typedef typename DataTypes::VecDeriv VecDeriv;
    typedef typename DataTypes::Coord    Coord   ;
    typedef typename DataTypes::Deriv    Deriv   ;
    typedef typename Coord::value_type   Real    ;

    typedef BaseMeshTopology::EdgeID ElementID;
    typedef vector<BaseMeshTopology::EdgeID> VecElementID;

    typedef MechanicalStateController<DataTypes> Inherit;

    typedef typename SolidTypes<Real>::Transform Transform;
    typedef typename SolidTypes<Real>::SpatialVector SpatialVector;
    typedef BeamInterpolation<DataTypes> BInterpolation;

    typedef Vec<3, Real> Vec3;

public :
    AdaptiveBeamController();
    virtual ~AdaptiveBeamController(){}


    /////////////// Inherited from PointActiver ////////////////////////////////////////////////////
    virtual bool activePoint(int index, core::CollisionModel *cm = nullptr) override ;


    /////////////// Inherited from LineActiver /////////////////////////////////////////////////////
    virtual bool activeLine(int index, core::CollisionModel *cm = nullptr) override ;


    /////////////// Inherited from BaseObject  /////////////////////////////////////////////////////
    virtual void init() override ;
    virtual void reinit() override ;

    /////////////// Inherited from MechanicalStateController  //////////////////////////////////////
    virtual void onMouseEvent(core::objectmodel::MouseEvent *) override ;
    virtual void onKeyPressedEvent(core::objectmodel::KeypressedEvent *) override ;
    virtual void onBeginAnimationStep(const double dt) override ;

    // TODO(dmarchal 2017-05-17) to eulalie & christian (remove 1 one year if not answered)
    // there is a non-virtual function in MechanicalStateControlller but it is non virtual..
    // so so should applyController really be a virtual metho ?
    virtual void applyController(void) /* override */ ;

    // TODO(dmarchal 2017-05-17) to eulalie & christian (remove 1 one year if not answered)
    // there is a non-virtual function in MechanicalStateControlller with this name and it is not
    // not used do we need this  function ?
    virtual bool modifyTopology(void) /* override */ { return false;}


    //TODO(dmarchal 2017-05-17) Check that these two are really needed (remove 1 one year if not done)
    virtual std::string getTemplateName() const
    {
      return templateName(this);
    }

    static std::string templateName(const AdaptiveBeamController<DataTypes>* = NULL)
    {
      return DataTypes::Name();
    }


protected:
    Data<vector<std::string>>   d_interpolationPath;
    Data<int>                   d_controlledInstrument;
    Data<vector<Real>>          d_xtip;
    Data<vector<Real>>          d_rotationInstrument;
    Data<Real>                  d_step;
    Data<Real>                  d_angularStep;
    Data<Real>                  d_speed;
    Data<Coord>                 d_startingPos;
    Data<Real>                  d_threshold;

    BInterpolation*             m_adaptiveinterpolation {nullptr};

    ////// for point and line activer
    vector<Real>                m_xAbs_collisionPoints_buf;

    bool                        FF {false} ;
    bool                        RW {false} ;

    FixedConstraint<DataTypes>* m_fixedConstraint {nullptr} ;
    vector<int>                 m_droppedInstruments ;

    vector<Real>                m_nodeCurvAbs ;
    vector<vector<int>>         m_id_instrument_curvAbs_table ;
    unsigned int                m_numControlledNodes {0};  // excluding the nodes that are "dropped"

    bool                        m_dropCall {false};

    /////////// Interface for other Adaptive Control
    BaseMeshTopology*                     m_topology {nullptr} ;
    EdgeSetGeometryAlgorithms<DataTypes>* m_edgeGeo {nullptr} ;
    EdgeSetTopologyModifier*              m_edgeMod {nullptr} ;
    Coord                                 m_refPos;
    vector<Real>                          m_vertexT; //=> replace by curvilinearAbs;
    Real                                  m_edgeTLength {0} ;
};

} // namespace _adaptivebeamcontroller_

/// 'Export' the objects defined in the private namespace into the 'public' one.
using _adaptivebeamcontroller_::AdaptiveBeamController ;

} // namespace controller

} // namespace component

} // namespace sofa

#endif /* SOFA_COMPONENT_CONTROLLER_ADAPTIVEBEAMCONTROLLER_H */
