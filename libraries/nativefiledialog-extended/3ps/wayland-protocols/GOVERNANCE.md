# wayland-protocols governance

This document governs the maintenance of wayland-protocols and serves to outline
the broader process for standardization of protocol extensions in the Wayland
ecosystem.

## 1. Membership

Membership in wayland-protocols is offered to stakeholders in the Wayland
ecosystem who have an interest in participating in protocol extension
standardization.

### 1.1. Membership requirements

1. Membership is extended to projects, rather than individuals.
2. Member projects represent general-purpose projects with a stake in multiple
   Wayland protocols (e.g. compositors, GUI toolkits, etc), rather than
   special-purpose applications with a stake in only one or two.
3. Each member project must provide named individuals as point of contact
   for that project who can be reached to discuss protocol-related matters.
4. During a vote, if any points-of-contact for the same member project
   disagree, the member project's vote is considered blank.

### 1.2. Becoming a member

1. New member projects who meet the criteria outlined in 1.1 are established by
   invitation from an existing member. Projects hoping to join should reach out
   to an existing member project or point of contact, asking for this
   invitation.
2. Prospective new member projects shall file a merge request tagged
   `governance` adding themselves to the list in `MEMBERS.md`, noting their
   sponsor member.
3. A point of contact for the sponsor member shall respond acknowledging their
   sponsorship of the membership.
4. A 14 day discussion period for comments from wayland-protocols members will
   be held.
5. At the conclusion of the discussion period, the new membership is
   established unless their application was NACKed by a 1/2 majority of all
   existing member projects.
6. Member projects may vary their point(s) of contact by proposing the addition
   and/or removal of points of contact in a merge request tagged `governance`,
   subject to approval as in points 4 and 5 above.

### 1.3. Ceasing membership

1. A member project, or point of contact, may step down by submitting a merge
   request tagged `governance` removing themselves from `MEMBERS.md`.
2. A removal vote may be called for by an existing member project or point of
   contact, by filing a merge request tagged `governance`, removing the
   specific member project or point of contact from `MEMBERS.md`. This begins a
   14 day voting & discussion period.
3. At the conclusion of the voting period, the member is removed if the votes
   total 2/3rds of all current member projects.
4. Removed members are not eligible to apply for membership again for a period
   of 1 year.
5. Following a failed vote, the member project who called for the vote cannot
   call for a re-vote or propose any other removal for 90 days.

## 2. Protocols

### 2.1. Protocol namespaces

1. Namespaces are implemented in practice by prefixing each interface name in a
   protocol definition (XML) with the namespace name, and an underscore (e.g.
   "xdg_wm_base").
2. Protocols in a namespace may optionally use the namespace followed by a dash
   in the name (e.g. "xdg-shell").
3. The "xdg" namespace is established for protocols letting clients
   configure their surfaces as "windows", allowing clients to affect how they
   are managed.
4. The "wp" namespace is established for protocols generally useful to Wayland
   implementations (i.e. "plumbing" protocols).
5. The "ext" namespace is established as a general catch-all for protocols that
   fit into no other namespace.

#### 2.1.1 Experimental protocol namespacing

1. Experimental protocols begin with the "xx" namespace and do not include any relation
   to namespaces specified in section 2.1.
2. Namespacing of experimental protocols is determined upon promotion.

### 2.2. Protocol inclusion requirements

1. All protocols found in the "xdg" and "wp" namespaces at the time of writing
   are grandfathered into their respective namespace without further discussion.
2. Protocols in the "xdg" and "wp" namespace are eligible for inclusion only if
   ACKed by at least 3 members.
3. Protocols in the "xdg" and "wp" namespace are ineligible for inclusion if
   NACKed by any member.
4. Protocols in the "xdg" and "wp" namespaces must have at least 3 open-source
   implementations (either 1 client + 2 servers, or 2 clients + 1 server) to be
   eligible for inclusion.
5. Protocols in the "ext" namespace are eligible for inclusion only if ACKed by
   at least 2 member projects.
6. Protocols in the "ext" namespace must have at least one open-source client &
   one open-source server implementation to be eligible for inclusion.
7. "Open-source" is defined as distributed with an Open Source Initiative
   approved license.
8. All protocols are eligible for inclusion only if formally reviewed in-depth
   by at least one member project. For the purposes of this clause, reviews from
   the individual protocol author(s) are disregarded.

#### 2.2.1 Experimental protocol inclusion requirements

1. Experimental protocols must be valid XML which can be consumed by wayland-scanner.
2. All such protocols must be created with a proposal merge request outlining the
   need for and purpose of the protocol.
3. All such protocols must be clearly tagged as experimental.

### 2.3. Introducing new protocols

1. A new protocol may be proposed by submitting a merge request to the
   wayland-protocols Gitlab repository.
2. Protocol proposal posts must include justification for their inclusion in
   their namespace per the requirements outlined in section 2.2.
3. An indefinite discussion period for comments from wayland-protocols members
   will be held, with a minimum duration of 30 days beginning from the time when
   the MR was opened. Protocols which require a certain level of implementation
   status, ACKs from members, and so on, should use this time to acquire them.
4. When the proposed protocol meets all requirements for inclusion per section
   2.2, and the minimum discussion period has elapsed, the sponsoring member may
   merge their changes into the wayland-protocol repository.
5. Amendments to existing protocols may be proposed by the same process, with
   no minimum discussion period.
6. Declaring a protocol stable may be proposed by the same process, with the
   regular 30 day minimum discussion period.
7. A member project has the option to invoke the 30 day discussion period for any
   staging protocol proposal which has been in use without substantive changes
   for a period of one year.

### 2.4. Development stalemate resolution

1. In the event that a discussion thread reaches a stalemate which cannot be
   resolved, a tie-breaking vote can be requested by the protocol author or
   any member project.
2. All member projects are eligible to vote in stalemate tie-breakers. Each project
   may cast a single vote.
3. Tie-breaker voting periods last no fewer than seven days.
4. Tie-breaker votes must be between two choices.
5. Any member project may elect to extend the voting period by an additional seven days.
   This option may only be invoked once per member project per tie-breaker and shall
   not be used without cause.
6. At the end of the voting period, the choice with the most votes is declared
   the winner, and development proceeds using that idea.
7. In the event of a tie, the protocol author casts the deciding vote.

### 2.5. Representation of non-members

1. A protocol proposed by a non-member inherently begins at a
   responsibility deficit as compared to one initiated by a member project.
2. To address this, any protocol proposed by a non-member intended for `staging/` or
   `stable/` may have a sponsor designated from a member project
3. The sponsor should have a strong understanding of the protocol they
   represent as well as the time required to drive it.
4. The sponsor shall be responsible for representing the protocol and its
   author in all cases which require membership, e.g., stalemate voting.
5. The member projects shall provide a sponsor for a non-member project upon request.
6. An author may make a one-time request for a different sponsor at any point.

### 2.3.1 Introducing new experimental protocols

1. Experimental protocols are merged into wayland-protocols after a two
   week review period upon the author's request unless a NACK has been given or
   a WAIT is in progress.
2. If all NACKs are removed from an experimental protocol, the two week review period is
   started anew.

### 2.3.2 Experimental protocol removal policy

1. Unmaintained experimental protocols are removed after a three month period of
   inactivity by its author, as determined by all of the following being true:
   * No changes have been made to the protocol by the author
   * No comments have been made to the protocol merge request by the author
   * No mails have been sent to the mailing list persuant to the protocol by the author
2. A notification of intent to remove shall be given to the author in the protocol
   merge request, and the protocol shall only be removed following a one week grace period
   of continued inactivity.

### 2.3.3 Experimental protocol promotion

1. A merged experimental protocol may be promoted to `staging/`
   upon request if it meets the requirements for landing as a
   `staging/` protocol.
2. Upon promotion, an experimental protocol is removed from `experimental/`.

## 3. NACKs

1. Expressing a NACK is the sole purview of listed points-of-contact from member projects,
   as specified in MEMBERS.md.
   A NACK must be grounded in technical reasoning, and it constitutes the final resort
   to block protocols which would harm the ecosystem or the project.
2. Any non-point-of-contact mentioning a NACK on a non-governance protocol issue, merge request,
   or mailing list thread, for any purpose, shall be banned from the project for a
   period of no fewer than three months. Additional penalties for repeat infractions
   may be imposed at the discretion of a membership majority. A warning, delivered in private
   if at all possible, shall be issued instead of a ban for first-time violations of this rule.
   Any comments violating this rule shall be explicitly marked by member projects to indicate that
   the NACK is invalid and has no bearing.
3. Any member project mentioning a NACK on a non-governance protocol issue, merge request,
   or mailing list thread, for any reason that may be considered non-technical,
   may undergo trial by eligible member projects upon receiving a written accusation of
   impropriety. This accusation may be public or private, and it may occur by any method
   of written communication.
   If this NACK is determined by 2/3 majority of eligible member projects to be used improperly,
   the offending point-of-contact shall be removed.
4. Eligible member projects during such review periods are those who have opted not to recuse themselves.

## 4. Amending this document

1. An amendment to this document may be proposed by any member project by
   submitting a merge request on Gitlab.
2. A 30 day discussion period for comments from wayland-protocols members will
   be held.
3. At the conclusion of the discussion period, an amendment will become
   effective if it's ACKed by at least 2/3rds of all wayland-protocols member
   projects, and NACKed by none. The sponsoring member may merge their change
   to the wayland-protocols repository at this point.
