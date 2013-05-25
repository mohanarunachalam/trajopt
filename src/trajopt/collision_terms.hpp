#pragma once
#include "trajopt/common.hpp"
#include "trajopt/collision_checker.hpp"
#include "sco/modeling.hpp"
#include "sco/sco_fwd.hpp"
#include "cache.hxx"


namespace trajopt {

typedef std::map<const OR::KinBody::Link*, int> Link2Int;


struct CollisionEvaluator {
  virtual void CalcDistExpressions(const DblVec& x, vector<AffExpr>& exprs, DblVec& weights) = 0;
  virtual void CalcDists(const DblVec& x, DblVec& exprs, DblVec& weights) = 0;
  virtual void CalcCollisions(const DblVec& x, vector<Collision>& collisions) = 0;
  void GetCollisionsCached(const DblVec& x, vector<Collision>&);
  virtual ~CollisionEvaluator() {}

  Cache<double, vector<Collision>, 3> m_cache;
};
typedef boost::shared_ptr<CollisionEvaluator> CollisionEvaluatorPtr;

struct SingleTimestepCollisionEvaluator : public CollisionEvaluator {
public:
  SingleTimestepCollisionEvaluator(ConfigurationPtr rad, const VarVector& vars);
  /**
  @brief linearize all contact distances in terms of robot dofs
  
  Do a collision check between robot and environment.
  For each contact generated, return a linearization of the signed distance function
  Since the collision checker may return multiple contact points for a given pair of links,
  the contacts are associated with weights so that each pair of links are associated with a single cost term.
  In particular, if a pair of bodies have k contacts, then the contacts each have weight 1/k.
  */
  void CalcDistExpressions(const DblVec& x, vector<AffExpr>& exprs, DblVec& weights); 
  /**
   * Same as CalcDistExpressions, but just the distances--not the expressions
   */
  void CalcDists(const DblVec& x, DblVec& exprs, DblVec& weights); 
  void CalcCollisions(const DblVec& x, vector<Collision>& collisions);

  OR::EnvironmentBasePtr m_env;
  CollisionCheckerPtr m_cc;
  ConfigurationPtr m_rad;
  VarVector m_vars;
  Link2Int m_link2ind;
  vector<OR::KinBody::LinkPtr> m_links;
  short m_filterMask;
};

struct CastCollisionEvaluator : public CollisionEvaluator {
public:
  CastCollisionEvaluator(ConfigurationPtr rad, const VarVector& vars0, const VarVector& vars1);
  void CalcDistExpressions(const DblVec& x, vector<AffExpr>& exprs, DblVec& weights);
  void CalcDists(const DblVec& x, DblVec& exprs, DblVec& weights);
  void CalcCollisions(const DblVec& x, vector<Collision>& collisions);

  // parameters:
  OR::EnvironmentBasePtr m_env;
  CollisionCheckerPtr m_cc;
  ConfigurationPtr m_rad;
  VarVector m_vars0;
  VarVector m_vars1;
  typedef std::map<const OR::KinBody::Link*, int> Link2Int;
  Link2Int m_link2ind;
  vector<OR::KinBody::LinkPtr> m_links;
  short m_filterMask;

};


class TRAJOPT_API CollisionCost : public Cost {
public:
  /* constructor for single timestep */
  CollisionCost(double dist_pen, double coeff, ConfigurationPtr rad, const VarVector& vars);
  /* constructor for cast cost */
  CollisionCost(double dist_pen, double coeff, ConfigurationPtr rad, const VarVector& vars0, const VarVector& vars1);
  virtual ConvexObjectivePtr convex(const vector<double>& x, Model* model);
  virtual double value(const vector<double>&);
private:
  CollisionEvaluatorPtr m_calc;
  double m_dist_pen;
  double m_coeff;
};
class TRAJOPT_API CollisionConstraint : public IneqConstraint {
public:
  /* constructor for single timestep */
  CollisionConstraint(double dist_pen, double coeff, ConfigurationPtr rad, const VarVector& vars);
  /* constructor for cast cost */
  CollisionConstraint(double dist_pen, double coeff, ConfigurationPtr rad, const VarVector& vars0, const VarVector& vars1);
  virtual ConvexConstraintsPtr convex(const vector<double>& x, Model* model);
  virtual DblVec value(const vector<double>&);
private:
  CollisionEvaluatorPtr m_calc;
  double m_dist_pen;
  double m_coeff;
};

}