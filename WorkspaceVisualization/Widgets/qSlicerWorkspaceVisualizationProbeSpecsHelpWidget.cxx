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
#include "qSlicerWorkspaceVisualizationProbeSpecsHelpWidget.h"
#include "ui_qSlicerWorkspaceVisualizationProbeSpecsHelpWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_WorkspaceVisualization
class qSlicerWorkspaceVisualizationProbeSpecsHelpWidgetPrivate
  : public Ui_qSlicerWorkspaceVisualizationProbeSpecsHelpWidget
{
  Q_DECLARE_PUBLIC(qSlicerWorkspaceVisualizationProbeSpecsHelpWidget);

protected:
  qSlicerWorkspaceVisualizationProbeSpecsHelpWidget* const q_ptr;

public:
  qSlicerWorkspaceVisualizationProbeSpecsHelpWidgetPrivate(
    qSlicerWorkspaceVisualizationProbeSpecsHelpWidget& object);
  virtual void setupUi(qSlicerWorkspaceVisualizationProbeSpecsHelpWidget*);
};

// --------------------------------------------------------------------------
qSlicerWorkspaceVisualizationProbeSpecsHelpWidgetPrivate ::
  qSlicerWorkspaceVisualizationProbeSpecsHelpWidgetPrivate(
    qSlicerWorkspaceVisualizationProbeSpecsHelpWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerWorkspaceVisualizationProbeSpecsHelpWidgetPrivate ::setupUi(
  qSlicerWorkspaceVisualizationProbeSpecsHelpWidget* widget)
{
  this->Ui_qSlicerWorkspaceVisualizationProbeSpecsHelpWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerWorkspaceVisualizationProbeSpecsHelpWidget methods

//-----------------------------------------------------------------------------
qSlicerWorkspaceVisualizationProbeSpecsHelpWidget ::
  qSlicerWorkspaceVisualizationProbeSpecsHelpWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qSlicerWorkspaceVisualizationProbeSpecsHelpWidgetPrivate(*this))
{
  Q_D(qSlicerWorkspaceVisualizationProbeSpecsHelpWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerWorkspaceVisualizationProbeSpecsHelpWidget ::
  ~qSlicerWorkspaceVisualizationProbeSpecsHelpWidget()
{
}
