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
#include "qSlicerWorkspaceGenerationProbeSpecsHelpWidget.h"
#include "ui_qSlicerWorkspaceGenerationProbeSpecsHelpWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_WorkspaceGeneration
class qSlicerWorkspaceGenerationProbeSpecsHelpWidgetPrivate
  : public Ui_qSlicerWorkspaceGenerationProbeSpecsHelpWidget
{
  Q_DECLARE_PUBLIC(qSlicerWorkspaceGenerationProbeSpecsHelpWidget);

protected:
  qSlicerWorkspaceGenerationProbeSpecsHelpWidget* const q_ptr;

public:
  qSlicerWorkspaceGenerationProbeSpecsHelpWidgetPrivate(
    qSlicerWorkspaceGenerationProbeSpecsHelpWidget& object);
  virtual void setupUi(qSlicerWorkspaceGenerationProbeSpecsHelpWidget*);
};

// --------------------------------------------------------------------------
qSlicerWorkspaceGenerationProbeSpecsHelpWidgetPrivate ::
  qSlicerWorkspaceGenerationProbeSpecsHelpWidgetPrivate(
    qSlicerWorkspaceGenerationProbeSpecsHelpWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerWorkspaceGenerationProbeSpecsHelpWidgetPrivate ::setupUi(
  qSlicerWorkspaceGenerationProbeSpecsHelpWidget* widget)
{
  this->Ui_qSlicerWorkspaceGenerationProbeSpecsHelpWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerWorkspaceGenerationProbeSpecsHelpWidget methods

//-----------------------------------------------------------------------------
qSlicerWorkspaceGenerationProbeSpecsHelpWidget ::
  qSlicerWorkspaceGenerationProbeSpecsHelpWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qSlicerWorkspaceGenerationProbeSpecsHelpWidgetPrivate(*this))
{
  Q_D(qSlicerWorkspaceGenerationProbeSpecsHelpWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerWorkspaceGenerationProbeSpecsHelpWidget ::
  ~qSlicerWorkspaceGenerationProbeSpecsHelpWidget()
{
}
