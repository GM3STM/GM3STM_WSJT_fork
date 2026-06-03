#include "OmniRig.h"

#include <OAIdl.h>

OmniRig::IOmniRigX::IOmniRigX (IDispatch * subobject, QAxObject * parent)
  : QAxObject {static_cast<IUnknown *> (subobject), parent}
{
  internalRelease ();
}

OmniRig::IOmniRigXEvents::IOmniRigXEvents (IDispatch * subobject, QAxObject * parent)
  : QAxObject {static_cast<IUnknown *> (subobject), parent}
{
  internalRelease ();
}

OmniRig::IRigX::IRigX (IDispatch * subobject, QAxObject * parent)
  : QAxObject {static_cast<IUnknown *> (subobject), parent}
{
  internalRelease ();
}

OmniRig::IPortBits::IPortBits (IDispatch * subobject, QAxObject * parent)
  : QAxObject {static_cast<IUnknown *> (subobject), parent}
{
  internalRelease ();
}

OmniRig::OmniRigX::OmniRigX (QObject * parent)
  : QAxObject {parent}
{
  setControl (QStringLiteral ("{0839e8c6-ed30-4950-8087-966f970f0cae}"));
}

OmniRig::RigX::RigX (QObject * parent)
  : QAxObject {parent}
{
  setControl (QStringLiteral ("{78aecfa2-3f52-4e39-98d3-1646c00a6234}"));
}

OmniRig::PortBits::PortBits (QObject * parent)
  : QAxObject {parent}
{
  setControl (QStringLiteral ("{b786de29-3b3d-4c66-b7c4-547f9a77a21d}"));
}
