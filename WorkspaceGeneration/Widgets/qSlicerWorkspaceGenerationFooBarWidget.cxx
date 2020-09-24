/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// FooBar Widgets includes
#include "qSlicerWorkspaceGenerationFooBarWidget.h"
#include "ui_qSlicerWorkspaceGenerationFooBarWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_WorkspaceGeneration
class qSlicerWorkspaceGenerationFooBarWidgetPrivate
  : public Ui_qSlicerWorkspaceGenerationFooBarWidget
{
  Q_DECLARE_PUBLIC(qSlicerWorkspaceGenerationFooBarWidget);
protected:
  qSlicerWorkspaceGenerationFooBarWidget* const q_ptr;

public:
  qSlicerWorkspaceGenerationFooBarWidgetPrivate(
    qSlicerWorkspaceGenerationFooBarWidget& object);
  virtual void setupUi(qSlicerWorkspaceGenerationFooBarWidget*);
};

// --------------------------------------------------------------------------
qSlicerWorkspaceGenerationFooBarWidgetPrivate
::qSlicerWorkspaceGenerationFooBarWidgetPrivate(
  qSlicerWorkspaceGenerationFooBarWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerWorkspaceGenerationFooBarWidgetPrivate
::setupUi(qSlicerWorkspaceGenerationFooBarWidget* widget)
{
  this->Ui_qSlicerWorkspaceGenerationFooBarWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerWorkspaceGenerationFooBarWidget methods

//-----------------------------------------------------------------------------
qSlicerWorkspaceGenerationFooBarWidget
::qSlicerWorkspaceGenerationFooBarWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerWorkspaceGenerationFooBarWidgetPrivate(*this) )
{
  Q_D(qSlicerWorkspaceGenerationFooBarWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerWorkspaceGenerationFooBarWidget
::~qSlicerWorkspaceGenerationFooBarWidget()
{
}
