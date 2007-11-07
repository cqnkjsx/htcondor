/***************************************************************
 *
 * Copyright (C) 1990-2007, Condor Team, Computer Sciences Department,
 * University of Wisconsin-Madison, WI.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License.  You may
 * obtain a copy of the License at
 * 
 *    http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************/


#ifndef _CONDOR_CLAIMID_PARSER_H
#define _CONDOR_CLAIMID_PARSER_H

// ClaimIdParser knows how to extract parts of a claim id string.
// It expects the following format (generated by the starter's claim.C)
// "<ip:port>#...#secret_cookie"

class ClaimIdParser {
 public:
	ClaimIdParser() {}
	ClaimIdParser(char const *claim_id) {
		m_claim_id = claim_id;
	}
	void setClaimId(char const *claim_id) {
		m_claim_id = claim_id;
		m_sinful_part = "";
		m_public_part = "";
	}
	char const *claimId() {
		return m_claim_id.Value();
	}
	char const *startdSinfulAddr() {
		if( m_sinful_part.IsEmpty() ) {
			char const *str = m_claim_id.Value();
			char const *end = strchr(str,'#');
			int length = end ? end - str : 0;
			m_sinful_part.sprintf("%.*s",length,str);
		}
		return m_sinful_part.Value();
	}
	char const *publicClaimId() {
		if( m_public_part.IsEmpty() ) {
			char const *str = m_claim_id.Value();
			char const *end = strrchr(str,'#');
			int length = end ? end - str : 0;
			m_public_part.sprintf("%.*s#...",length,str);
		}
		return m_public_part.Value();
	}
 private:
	MyString m_claim_id;
	MyString m_public_part;
	MyString m_sinful_part;
};

#endif
