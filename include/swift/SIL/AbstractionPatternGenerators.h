//===--- AbstractionPatternGenerators.h -------------------------*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2017 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
//
// This file defines "generators" that can be used with an AbstractionPattern
// to do certain kinds of traversal without using callbacks.
// This can be useful when a traversal is required in parallel with
// some other traversal.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_SIL_ABSTRACTIONPATTERNGENERATORS_H
#define SWIFT_SIL_ABSTRACTIONPATTERNGENERATORS_H

#include "swift/SIL/AbstractionPattern.h"

namespace swift {
namespace Lowering {

/// A generator for traversing the formal function parameters of a type
/// while properly respecting variadic generics.
class FunctionParamGenerator {
  // The steady state of the generator.

  /// The abstraction pattern of the entire function type.  Set once
  /// during construction.
  AbstractionPattern origFunctionType;

  /// The list of all substituted parameters to traverse.  Set once
  /// during construction.
  AnyFunctionType::CanParamArrayRef allSubstParams;

  /// The number of orig parameters to traverse.  Set once during
  /// construction.
  unsigned numOrigParams;

  /// The index of the current orig parameter.
  /// Incremented during advance().
  unsigned origParamIndex = 0;

  /// The (start) index of the current subst parameters.
  /// Incremented during advance().
  unsigned substParamIndex = 0;

  /// The number of subst parameters corresponding to the current
  /// orig parameter.
  unsigned numSubstParamsForOrigParam;

  /// Whether the orig function type is opaque, i.e. does not permit us to
  /// call getNumFunctionParams() and similar accessors.  Set once during
  /// construction.
  bool origFunctionTypeIsOpaque;

  /// Whether the current orig parameter is a pack expansion.
  bool origParamIsExpansion;

  /// The abstraction pattern of the current orig parameter.
  /// If it is a pack expansion, this is the expansion type, not the
  /// pattern type.
  AbstractionPattern origParamType = AbstractionPattern::getInvalid();

  /// Load the informaton for the current orig parameter into the
  /// fields above for it.
  void loadParameter() {
    origParamType = origFunctionType.getFunctionParamType(origParamIndex);
    origParamIsExpansion = origParamType.isPackExpansion();
    numSubstParamsForOrigParam =
      (origParamIsExpansion
         ? origParamType.getNumPackExpandedComponents()
         : 1);
  }

public:
  FunctionParamGenerator(AbstractionPattern origFunctionType,
                         AnyFunctionType::CanParamArrayRef substParams,
                         bool ignoreFinalOrigParam);

  /// Is the traversal finished?  If so, none of the getters below
  /// are allowed to be called.
  bool isFinished() const {
    return origParamIndex == numOrigParams;
  }

  /// Advance to the next orig parameter.
  void advance() {
    assert(!isFinished());
    origParamIndex++;
    substParamIndex += numSubstParamsForOrigParam;
    if (!isFinished()) loadParameter();
  }

  /// Return the index of the current orig parameter.
  unsigned getOrigIndex() const {
    assert(!isFinished());
    return origParamIndex;
  }

  /// Return the index of the (first) subst parameter corresponding
  /// to the current orig parameter.
  unsigned getSubstIndex() const {
    assert(!isFinished());
    return origParamIndex;
  }

  /// Return the parameter flags for the current orig parameter.
  ParameterTypeFlags getOrigFlags() const {
    assert(!isFinished());
    return (origFunctionTypeIsOpaque
              ? allSubstParams[substParamIndex].getParameterFlags()
              : origFunctionType.getFunctionParamFlags(origParamIndex));
  }

  /// Return the type of the current orig parameter.
  const AbstractionPattern &getOrigType() const {
    assert(!isFinished());
    return origParamType;
  }

  /// Return whether the current orig parameter type is a pack expansion.
  bool isOrigPackExpansion() const {
    assert(!isFinished());
    return origParamIsExpansion;
  }

  /// Return the substituted parameters corresponding to the current
  /// orig parameter type.  If the current orig parameter is not a
  /// pack expansion, this will have exactly one element.
  AnyFunctionType::CanParamArrayRef getSubstParams() const {
    assert(!isFinished());
    return allSubstParams.slice(substParamIndex, numSubstParamsForOrigParam);
  }

  /// Call this to finalize the traversal and assert that it was done
  /// properly.
  void finish() {
    assert(isFinished() && "didn't finish the traversal");
    assert(substParamIndex == allSubstParams.size() &&
           "didn't exhaust subst parameters; possible missing subs on "
           "orig function type");
  }
};

/// A generator for traversing the formal elements of a tuple type
/// while properly respecting variadic generics.
class TupleElementGenerator {
  // The steady state of the generator.

  /// The abstraction pattern of the entire tuple type.  Set once
  /// during construction.
  AbstractionPattern origTupleType;

  /// The substitute tuple type.  Set once during construction.
  CanTupleType substTupleType;

  /// The number of orig elements to traverse.  Set once during
  /// construction.
  unsigned numOrigElts;

  /// The index of the current orig element.
  /// Incremented during advance().
  unsigned origEltIndex = 0;

  /// The (start) index of the current subst elements.
  /// Incremented during advance().
  unsigned substEltIndex = 0;

  /// The number of subst elements corresponding to the current
  /// orig element.
  unsigned numSubstEltsForOrigElt;

  /// Whether the orig tuple type is opaque, i.e. does not permit us to
  /// call getNumTupleElements() and similar accessors.  Set once during
  /// construction.
  bool origTupleTypeIsOpaque;

  /// Whether the current orig element is a pack expansion.
  bool origEltIsExpansion;

  /// The abstraction pattern of the current orig element.
  /// If it is a pack expansion, this is the expansion type, not the
  /// pattern type.
  AbstractionPattern origEltType = AbstractionPattern::getInvalid();

  /// Load the informaton for the current orig element into the
  /// fields above for it.
  void loadElement() {
    origEltType = origTupleType.getTupleElementType(origEltIndex);
    origEltIsExpansion = origEltType.isPackExpansion();
    numSubstEltsForOrigElt =
      (origEltIsExpansion
         ? origEltType.getNumPackExpandedComponents()
         : 1);
  }

public:
  TupleElementGenerator(AbstractionPattern origTupleType,
                        CanTupleType substTupleType);

  /// Is the traversal finished?  If so, none of the getters below
  /// are allowed to be called.
  bool isFinished() const {
    return origEltIndex == numOrigElts;
  }

  /// Advance to the next orig element.
  void advance() {
    assert(!isFinished());
    origEltIndex++;
    substEltIndex += numSubstEltsForOrigElt;
    if (!isFinished()) loadElement();
  }

  /// Return the index of the current orig element.
  unsigned getOrigIndex() const {
    assert(!isFinished());
    return origEltIndex;
  }

  /// Return the index of the (first) subst element corresponding
  /// to the current orig element.
  unsigned getSubstIndex() const {
    assert(!isFinished());
    return origEltIndex;
  }

  /// Return a tuple element for the current orig element.
  TupleTypeElt getOrigElement() const {
    assert(!isFinished());
    return (origTupleTypeIsOpaque
              ? substTupleType->getElement(substEltIndex)
              : cast<TupleType>(origTupleType.getType())
                  ->getElement(origEltIndex));
  }

  /// Return the type of the current orig element.
  const AbstractionPattern &getOrigType() const {
    assert(!isFinished());
    return origEltType;
  }

  /// Return whether the current orig element type is a pack expansion.
  bool isOrigPackExpansion() const {
    assert(!isFinished());
    return origEltIsExpansion;
  }

  /// Return the substituted elements corresponding to the current
  /// orig element type.  If the current orig element is not a
  /// pack expansion, this will have exactly one element.
  CanTupleEltTypeArrayRef getSubstTypes() const {
    assert(!isFinished());
    return substTupleType.getElementTypes().slice(substEltIndex,
                                                  numSubstEltsForOrigElt);
  }

  /// Call this to finalize the traversal and assert that it was done
  /// properly.
  void finish() {
    assert(isFinished() && "didn't finish the traversal");
    assert(substEltIndex == substTupleType->getNumElements() &&
           "didn't exhaust subst elements; possible missing subs on "
           "orig tuple type");
  }
};

} // end namespace Lowering
} // end namespace swift

#endif
