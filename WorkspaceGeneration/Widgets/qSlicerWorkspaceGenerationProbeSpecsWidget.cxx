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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware
Inc. and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// ProbeSpecs Widgets includes
#include "qSlicerWorkspaceGenerationProbeSpecsWidget.h"
#include "ui_qSlicerWorkspaceGenerationProbeSpecsWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_WorkspaceGeneration
class qSlicerWorkspaceGenerationProbeSpecsWidgetPrivate
  : public Ui_qSlicerWorkspaceGenerationProbeSpecsWidget
{
  Q_DECLARE_PUBLIC(qSlicerWorkspaceGenerationProbeSpecsWidget);

protected:
  qSlicerWorkspaceGenerationProbeSpecsWidget* const q_ptr;

public:
  qSlicerWorkspaceGenerationProbeSpecsWidgetPrivate(
    qSlicerWorkspaceGenerationProbeSpecsWidget& object);
  virtual void setupUi(qSlicerWorkspaceGenerationProbeSpecsWidget*);
};

// --------------------------------------------------------------------------
qSlicerWorkspaceGenerationProbeSpecsWidgetPrivate ::
  qSlicerWorkspaceGenerationProbeSpecsWidgetPrivate(
    qSlicerWorkspaceGenerationProbeSpecsWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerWorkspaceGenerationProbeSpecsWidgetPrivate ::setupUi(
  qSlicerWorkspaceGenerationProbeSpecsWidget* widget)
{
  this->Ui_qSlicerWorkspaceGenerationProbeSpecsWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerWorkspaceGenerationProbeSpecsWidget methods

//-----------------------------------------------------------------------------
qSlicerWorkspaceGenerationProbeSpecsWidget ::
  qSlicerWorkspaceGenerationProbeSpecsWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qSlicerWorkspaceGenerationProbeSpecsWidgetPrivate(*this))
{
  Q_D(qSlicerWorkspaceGenerationProbeSpecsWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerWorkspaceGenerationProbeSpecsWidget ::
  ~qSlicerWorkspaceGenerationProbeSpecsWidget()
{
}
