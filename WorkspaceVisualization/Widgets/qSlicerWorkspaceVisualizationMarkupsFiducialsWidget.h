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

#ifndef __qSlicerWorkspaceVisualizationMarkupsFiducialsWidget_h
#define __qSlicerWorkspaceVisualizationMarkupsFiducialsWidget_h

// Qt includes
#include <QWidget>

// MarkupsFiducials Widgets includes
#include "qSlicerWorkspaceVisualizationModuleWidgetsExport.h"

class qSlicerWorkspaceVisualizationMarkupsFiducialsWidgetPrivate;

/// \ingroup Slicer_QtModules_WorkspaceVisualization
class Q_SLICER_MODULE_WORKSPACEGENERATION_WIDGETS_EXPORT
  qSlicerWorkspaceVisualizationMarkupsFiducialsWidget : public QWidget
{
  Q_OBJECT
public:
  typedef QWidget Superclass;
  qSlicerWorkspaceVisualizationMarkupsFiducialsWidget(QWidget* parent = 0);
  virtual ~qSlicerWorkspaceVisualizationMarkupsFiducialsWidget();

protected slots:

protected:
  QScopedPointer< qSlicerWorkspaceVisualizationMarkupsFiducialsWidgetPrivate >
    d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerWorkspaceVisualizationMarkupsFiducialsWidget);
  Q_DISABLE_COPY(qSlicerWorkspaceVisualizationMarkupsFiducialsWidget);
};

#endif
