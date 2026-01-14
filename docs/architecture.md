1. Client
Responsibility

Provide a simple API for users or tests

Hide all distributed logic from the caller

What the Client Does

Sends:

PUT(key, value)

GET(key)

Talks to any node

Handles retries on failure



2. Storage Node (Process Boundary)

Each EC2 instance runs one storage node.

Responsibilities

Accept client requests

Route requests to correct replicas

Enforce quorum rules

Store local data

Communicate with peers

3. API / RPC Layer
Responsibility

Convert network requests ↔ internal calls

4. Request Router
Responsibility

Pure decision logic.

Given a key:

Decide which nodes are responsible

Inputs

Key

Cluster membership (static list)

5. Replication Coordinator
Responsibility

This is the brain of the system.

It:

Executes PUT/GET across replicas

Tracks responses

Enforces quorum rules

Triggers read repair

PUT Flow

Send PUT to N replicas

Wait for W responses

Decide success/failure

GET Flow

Query R replicas

Choose latest version

Trigger read repair if needed

6. Local Storage Engine
Responsibility

Store and retrieve local key–value data

Maintain version metadata

What It Does

In-memory map

Thread-safe access

Version comparison

7. Failure Detector (Minimal, Embedded)
Responsibility

Detect unresponsive peers

V1 Implementation

Timeout-based detection

No gossip

No membership change