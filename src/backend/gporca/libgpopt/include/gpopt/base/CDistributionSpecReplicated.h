//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2011 EMC Corp.
//
//	@filename:
//		CDistributionSpecReplicated.h
//
//	@doc:
//		Description of a replicated distribution;
//		Can be used as required or derived property;
//---------------------------------------------------------------------------
#ifndef GPOPT_CDistributionSpecReplicated_H
#define GPOPT_CDistributionSpecReplicated_H

#include "gpos/base.h"

#include "gpopt/base/CDistributionSpec.h"

namespace gpopt
{
using namespace gpos;

class CDistributionSpecReplicated : public CDistributionSpec
{
private:
	CDistributionSpecReplicated(const CDistributionSpecReplicated &) = delete;

public:

	enum class EReplicatedType
	{
		ErtStrict,
		ErtTainted,
		ErtGeneral,
		ErtSentinel
	};

private:
	// replicated support
	EReplicatedType m_replicated;

public:
	// ctor
	CDistributionSpecReplicated(EReplicatedType replicated_type)
		: m_replicated(replicated_type)
	{
	}

	// accessor
	EDistributionType
	Edt() const override
	{
		switch (m_replicated)
		{
			case EReplicatedType::ErtGeneral:
				return CDistributionSpec::EdtReplicated;
			case EReplicatedType::ErtTainted:
				return CDistributionSpec::EdtTaintedReplicated;
			case EReplicatedType::ErtStrict:
				return CDistributionSpec::EdtStrictReplicated;
			default:
				GPOS_ASSERT(!"Replicated type must be General, Tainted, or Strict");
				return CDistributionSpec::EdtSentinel;
		}
	}

	// does this distribution satisfy the given one
	BOOL FSatisfies(const CDistributionSpec *pds) const override;

	// append enforcers to dynamic array for the given plan properties
	void AppendEnforcers(CMemoryPool *mp, CExpressionHandle &exprhdl,
						 CReqdPropPlan *prpp, CExpressionArray *pdrgpexpr,
						 CExpression *pexpr) override;

	EReplicatedType Ert() const
	{
		return m_replicated;
	}

	// return distribution partitioning type
	EDistributionPartitioningType
	Edpt() const override
	{
		return EdptNonPartitioned;
	}

	// print
	IOstream &
	OsPrint(IOstream &os) const override
	{
		switch (Edt())
		{
			case CDistributionSpec::EdtReplicated:
				os << "REPLICATED";
				break;
			case CDistributionSpec::EdtTaintedReplicated:
				os << "TAINTED REPLICATED";
				break;
			case CDistributionSpec::EdtStrictReplicated:
				os << "STRICT REPLICATED";
				break;
			default:
				GPOS_ASSERT(!"Replicated type must be General, Tainted, or Strict");
		}
		return os;
	}

	// conversion function
	static CDistributionSpecReplicated *
	PdsConvert(CDistributionSpec *pds)
	{
		GPOS_ASSERT(NULL != pds);
		GPOS_ASSERT(EdtStrictReplicated == pds->Edt() ||
					EdtReplicated == pds->Edt() ||
					EdtTaintedReplicated == pds->Edt());

		return dynamic_cast<CDistributionSpecReplicated *>(pds);
	}

};	// class CDistributionSpecReplicated

}  // namespace gpopt

#endif	// !GPOPT_CDistributionSpecReplicated_H

// EOF
