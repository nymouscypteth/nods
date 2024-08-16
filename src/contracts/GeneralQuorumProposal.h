using namespace QPI;

struct GQMPROP2
{
};

struct GQMPROP : public ContractBase
{
	//----------------------------------------------------------------------------
	// Define common types

	// Proposal data type. We don't support scalar votes, but multi-option voting.
	typedef ProposalDataV1<false> ProposalDataT;

	// Computors have right to propose and vote. There is one proposal slot per computor to make sure
	// that a proposal can never be blocked by no free slots.
	typedef ProposalAndVotingByComputors<NUMBER_OF_COMPUTORS> ProposersAndVotersT;

	// Proposal and voting storage type
	typedef ProposalVoting<ProposersAndVotersT, ProposalDataT> ProposalVotingT;

	struct Success_output
	{
		bool okay;
	};

	struct RevenueDonationEntry
	{
		id destinationPublicKey;
		sint64 millionthAmount;
		uint16 firstEpoch;
	};

	typedef array<RevenueDonationEntry, 128> RevenueDonationT;

private:
	//----------------------------------------------------------------------------
	// Define state
	ProposalVotingT proposals;
	RevenueDonationT revenueDonation;

	//----------------------------------------------------------------------------
	// Define private procedures and functions with input and output

	typedef RevenueDonationEntry _SetRevenueDonationEntry_input;
	typedef Success_output _SetRevenueDonationEntry_output;
	struct _SetRevenueDonationEntry_locals
	{
		uint64 idx;
	};

	PRIVATE_PROCEDURE_WITH_LOCALS(_SetRevenueDonationEntry)
		// Try to find public key for updating entry
		for (locals.idx = 0; locals.idx < state.revenueDonation.capacity(); ++locals.idx)
		{
			if (input.destinationPublicKey == state.revenueDonation.get(locals.idx).destinationPublicKey)
			{
				// update entry
				state.revenueDonation.set(locals.idx, input);
				output.okay = true;
				return;
			}
		}

		// Public key not in table -> add entry to empty slot (with zero public key)
		for (locals.idx = 0; locals.idx < state.revenueDonation.capacity(); ++locals.idx)
		{
			if (isZero(state.revenueDonation.get(locals.idx).destinationPublicKey))
			{
				// add entry
				state.revenueDonation.set(locals.idx, input);
				output.okay = true;
				return;
			}
		}
	_

public:
	//----------------------------------------------------------------------------
	// Define public procedures and functions with input and output

	typedef ProposalDataT SetProposal_input;
	typedef Success_output SetProposal_output;
	struct SetProposal_locals
	{
		uint32 i;
		sint64 millionthAmount;
	};

	PUBLIC_PROCEDURE_WITH_LOCALS(SetProposal)
		// TODO: Fee? Burn fee?

		// Check requirements for proposals in this contract
		switch (ProposalTypes::cls(input.type))
		{
		case ProposalTypes::Class::Transfer:
			// Check that amounts, which are in millionth, are in range of 0 (= 0%) to 1000000 (= 100%)
			for (locals.i = 0; locals.i < 4; ++locals.i)
			{
				locals.millionthAmount = input.transfer.amounts.get(locals.i);
				if (locals.millionthAmount < 0 || locals.millionthAmount > 1000000)
				{
					output.okay = false;
					return;
				}
			}
			break;

		case ProposalTypes::Class::Variable:
			// Proposals for setting a variable are not allowed at the moment (lack of meaning)
			output.okay = false;
			return;
		}

		// Try to set proposal (checks originators rights and general validity of input proposal)
		output.okay = qpi(state.proposals).setProposal(qpi.originator(), input);
	_


	struct GetProposalIndices_input
	{
		bit activeProposals;		// Set true to return indices of active proposals, false for proposals of prior epochs
		sint32 prevProposalIndex;   // Set -1 to start getting indices. If returned index array is full, call again with highest index returned.
	};
	struct GetProposalIndices_output
	{
		uint16 numOfIndices;		// Number of valid entries in indices. Call again if it is 64.
		array<uint16, 64> indices;	// Requested proposal indices. Valid entries are in range 0 ... (numOfIndices - 1).
	};

	PUBLIC_FUNCTION(GetProposalIndices)
		if (input.activeProposals)
		{
			// Return proposals that are open for voting in current epoch
			// (output is initalized with zeros by contract system)
			while ((input.prevProposalIndex = qpi(state.proposals).nextActiveProposalIndex(input.prevProposalIndex)) >= 0)
			{
				output.indices.set(output.numOfIndices, input.prevProposalIndex);
				++output.numOfIndices;
			}
		}
		else
		{
			// Return proposals of previous epochs not overwritten yet
			// (output is initalized with zeros by contract system)
			while ((input.prevProposalIndex = qpi(state.proposals).nextFinishedProposalIndex(input.prevProposalIndex)) >= 0)
			{
				output.indices.set(output.numOfIndices, input.prevProposalIndex);
				++output.numOfIndices;
			}
		}
	_


	struct GetProposal_input
	{
		uint16 proposalIndex;
	};
	struct GetProposal_output
	{
		bit okay;
		id proposerPubicKey;
		ProposalDataT proposal;
	};

	PUBLIC_FUNCTION(GetProposal)
		output.proposerPubicKey = qpi(state.proposals).proposerId(input.proposalIndex);
		output.okay = qpi(state.proposals).getProposal(input.proposalIndex, output.proposal);
	_


	typedef ProposalSingleVoteDataV1 Vote_input;
	typedef Success_output Vote_output;

	PUBLIC_PROCEDURE(Vote)
		// TODO: Fee? Burn fee?
		output.okay = qpi(state.proposals).vote(qpi.originator(), input);
	_


	struct GetVote_input
	{
		id voter;
		uint16 proposalIndex;
	};
	struct GetVote_output
	{
		bit okay;
		ProposalSingleVoteDataV1 vote;
	};

	PUBLIC_FUNCTION(GetVote)
		output.okay = qpi(state.proposals).getVote(
			input.proposalIndex,
			qpi(state.proposals).voterIndex(input.voter),
			output.vote);
	_


	struct GetVotingResults_input
	{
		uint16 proposalIndex;
	};
	struct GetVotingResults_output
	{
		bit okay;
		ProposalSummarizedVotingDataV1 results;
	};

	PUBLIC_FUNCTION(GetVotingResults)
		output.okay = qpi(state.proposals).getVotingSummary(
			input.proposalIndex, output.results);
	_


	typedef NoData GetRevenueDonation_input;
	typedef RevenueDonationT GetRevenueDonation_output;

	PUBLIC_FUNCTION(GetRevenueDonation)
		output = state.revenueDonation;
	_


    REGISTER_USER_FUNCTIONS_AND_PROCEDURES
        REGISTER_USER_FUNCTION(GetProposalIndices, 1);
        REGISTER_USER_FUNCTION(GetProposal, 2);
        REGISTER_USER_FUNCTION(GetVote, 3);
        REGISTER_USER_FUNCTION(GetVotingResults, 4);
        REGISTER_USER_FUNCTION(GetRevenueDonation, 5);

        REGISTER_USER_PROCEDURE(SetProposal, 1);
        REGISTER_USER_PROCEDURE(Vote, 2);
    _

		
	struct END_EPOCH_locals
	{
		sint32 proposalIndex;
		ProposalDataT proposal;
		ProposalSummarizedVotingDataV1 results;
		sint32 optionIndex;
		uint32 optionVotes;
		sint32 mostVotedOptionIndex;
		uint32 mostVotedOptionVotes;
		RevenueDonationEntry revenueDonationEntry;
		Success_output success;
	};

	END_EPOCH_WITH_LOCALS
		// Analyze transfer proposal results

		// Iterate all proposals that were open for voting in current epoch ...
		locals.proposalIndex = -1;
		while (locals.proposalIndex = qpi(state.proposals).nextActiveProposalIndex(locals.proposalIndex) >= 0)
		{
			if (qpi(state.proposals).getProposal(locals.proposalIndex, locals.proposal))
			{
				// ... and have transfer proposal type
				if (ProposalTypes::cls(locals.proposal.type) == ProposalTypes::Class::Transfer)
				{
					// Get voting results and check if conditions for proposal acceptance are met
					if (qpi(state.proposals).getVotingSummary(locals.proposalIndex, locals.results))
					{
						// The total number of votes needs to be at least the quorum
						if (locals.results.totalVotes >= QUORUM)
						{
							// Find most voted "change" option (option 0 is "no change")
							locals.mostVotedOptionIndex = 0;
							locals.mostVotedOptionVotes = 0;
							for (locals.optionIndex = 1; locals.optionIndex < locals.results.optionCount; ++locals.optionIndex)
							{
								locals.optionVotes = locals.results.optionVoteCount.get(locals.optionIndex);
								if (locals.mostVotedOptionVotes < locals.optionVotes)
								{
									locals.mostVotedOptionVotes = locals.optionVotes;
									locals.mostVotedOptionIndex = locals.optionIndex;
								}
							}

							// Option for changing status quo has been accepted?
							if (locals.mostVotedOptionVotes > QUORUM/2)
							{
								// Set in revenueDonation table
								locals.revenueDonationEntry.destinationPublicKey = locals.proposal.transfer.destination;
								locals.revenueDonationEntry.millionthAmount = locals.proposal.transfer.amounts.get(locals.optionIndex - 1);
								locals.revenueDonationEntry.firstEpoch = qpi.epoch() + 1;
								CALL(_SetRevenueDonationEntry, locals.revenueDonationEntry, locals.success);
								break;
							}
						}
					}
					break;
				}
			}
		}
	_
};
