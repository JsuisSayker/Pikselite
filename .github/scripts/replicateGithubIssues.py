#!/usr/bin/env python3

import os
import requests
import json
from dotenv import load_dotenv
import sys

# Load the environment variables from the env file passed as the first argument in the command line when running this script
ENV_FILE_PATH = sys.argv[1]
print(f"Loading environment variables from: {ENV_FILE_PATH}")
load_dotenv(ENV_FILE_PATH)

# # NOT IMPLEMENTED YET
DST_OWNER = os.getenv("DST_OWNER")
DST_REPO = os.getenv("DST_REPO")
DST_PROJECT_NUMBER = os.getenv("DST_PROJECT_NUMBER")
DST_PROJECT_ID = os.getenv("DST_PROJECT_ID")
# # --------------------------------

SRC_OWNER = os.getenv("SRC_OWNER")
SRC_REPO = os.getenv("SRC_REPO")


GITHUB_TOKEN = os.getenv("GITHUB_TOKEN")
GITHUB_PROJECT_ID = os.getenv("GITHUB_PROJECT_ID")
GITHUB_PROJECT_OWNER = os.getenv("GITHUB_PROJECT_OWNER")
GITHUB_PROJECT_NUMBER = os.getenv("GITHUB_PROJECT_NUMBER")

# BASE_DIR = os.path.dirname(os.path.abspath(__file__))
# history_dir = os.path.join(BASE_DIR, "history")


def create_repository_issue(title, body, repo_id):

    query = """
    mutation CreateIssue($input: CreateIssueInput!) {
      createIssue(input: $input) {
        issue {
          id
          number
          url
        }
      }
    }
    """
    variables = {
        "input": {
            "repositoryId": repo_id,
            "title": title,
            "body": body
        }
    }
    result = run_graphql(query, variables)
    issue = result["data"]["createIssue"]["issue"]
    print(f"ISSUE : {issue}\n")
    return issue["id"], issue["number"]


def add_issue_to_project(issue_node_id, projectId):

    query = """
    mutation AddIssueToProject($input: AddProjectV2ItemByIdInput!) {
      addProjectV2ItemById(input: $input) {
        item {
          id
        }
      }
    }
    """
    variables = {
        "input": {
            "projectId": projectId,
            "contentId": issue_node_id
        }
    }
    result = run_graphql(query, variables)
    print(result)
    return result["data"]["addProjectV2ItemById"]["item"]["id"]


def get_repository_id(owner, repository):

    url = "https://api.github.com/graphql"
    query = """
    query {
      repository(owner: "%s", name: "%s") {
        id
      }
    }
    """ % (owner, repository)

    headers = {
        "Authorization": f"Bearer {GITHUB_TOKEN}",
        "Content-Type": "application/json",
    }

    payload = {"query": query}

    response = requests.post(url, headers=headers, json=payload)
    response.raise_for_status()
    data = response.json()
    return data["data"]["repository"]["id"]


def create_issue_on_board(title, body, target_repo_id):
    issue_node_id, issue_number = create_repository_issue(title, body, target_repo_id)
    project_item_id = add_issue_to_project(issue_node_id, DST_PROJECT_ID)
    return project_item_id, issue_number, issue_node_id


# def get_project_details():
#     url = "https://api.github.com/graphql"
#     headers = {
#         "Authorization": f"bearer {GITHUB_TOKEN}",
#         "Content-Type": "application/json"
#     }
#     query = f"""
#     query {{
#       user(login: "{GITHUB_PROJECT_OWNER}") {{
#         projectV2(number: {GITHUB_PROJECT_NUMBER}) {{
#           id
#           title
#           fields(first: 100) {{
#             nodes {{
#               ... on ProjectV2Field {{
#                 id
#                 name
#                 dataType
#               }}
#               ... on ProjectV2IterationField {{
#                 id
#                 name
#                 configuration {{
#                   iterations {{
#                     id
#                     title
#                   }}
#                 }}
#               }}
#               ... on ProjectV2SingleSelectField {{
#                 id
#                 name
#                 dataType
#                 options {{
#                   id
#                   name
#                 }}
#               }}
#             }}
#           }}
#         }}
#       }}
#     }}
#     """
#     payload = {"query": query}
#     response = requests.post(url, headers=headers, json=payload)
#     response.raise_for_status()
#     return response.json()


def run_graphql(query, variables):
    url = "https://api.github.com/graphql"
    headers = {
        "Authorization": f"bearer {GITHUB_TOKEN}",
        "Content-Type": "application/json",
        "Accept": "application/vnd.github.starfox-preview+json"
    }
    response = requests.post(url, json={
        "query": query, "variables": variables}, headers=headers)
    response.raise_for_status()
    return response.json()


# def add_labels_to_issue(issue_number, labels):
#     url = f"https://api.github.com/repos/{GITHUB_PROJECT_OWNER}/{GITHUB_PROJECT_NAME}/issues/{issue_number}/labels"
#     headers = {
#         "Authorization": f"Bearer {GITHUB_TOKEN}",
#         "Accept": "application/vnd.github+json"
#     }
#     response = requests.post(url, headers=headers, json=labels)
#     response.raise_for_status()
#     return response.json()


# def assign_iteration_to_issue(issue_node_id, field_name, side_infos_dict):

#     query = """
#     mutation UpdateIterationField($input: UpdateProjectV2ItemFieldValueInput!) {
#       updateProjectV2ItemFieldValue(input: $input) {
#         projectV2Item {
#           id
#         }
#       }
#     }
#     """
#     iteration_option_id = ""
#     field_id = field_name["id"]
#     for current_iteration in field_name["configuration"]["iterations"]:
#         if iteration_option_id != "":
#             break
#         for active_iteration in side_infos_dict["Sprint"]:
#             if iteration_option_id != "":
#                 break
#             if (current_iteration["title"] == active_iteration["name"]) and active_iteration["state"] == "active":
#                 iteration_option_id = current_iteration["id"]

#     variables = {
#         "input": {
#             "projectId": GITHUB_PROJECT_ID,
#             "itemId": issue_node_id,
#             "fieldId": field_id,
#             "value": {"iterationId": iteration_option_id}
#         }
#     }
#     run_graphql(query, variables)


# def update_status_field(issue_node_id, field_name, actual_status):

#     query = """
#     mutation UpdateStatusField($input: UpdateProjectV2ItemFieldValueInput!) {
#       updateProjectV2ItemFieldValue(input: $input) {
#         projectV2Item {
#           id
#         }
#       }
#     }
#     """
#     field_id = field_name["id"]
#     comparative_status = actual_status.replace(" ", "").lower()

#     for current_status in field_name["options"]:
#         if (current_status["name"].replace(" ", "").lower()) == comparative_status:
#             option_node_id = current_status["id"]
#             break

#     variables = {
#         "input": {
#             "projectId": GITHUB_PROJECT_ID,
#             "itemId": issue_node_id,
#             "fieldId": field_id,
#             "value": {"singleSelectOptionId": actual_status}
#         }
#     }
#     run_graphql(query, variables)


# def update_infos(issue_node_id, side_infos_dict, items_id, user_id, issue_id, issue_number, created_sprint):
#     sprint_found = False
#     for field_name in items_id["data"]["user"]["projectV2"]["fields"]["nodes"]:
#         if field_name:
#             if field_name["name"] in side_infos_dict:
#                 sent_value = side_infos_dict[field_name["name"]]
#                 if field_name["name"] == "Sprint":
#                     assign_iteration_to_issue(issue_node_id, field_name, side_infos_dict)
#                     sprint_found = True
#                     continue
#                 key = field_name["dataType"].lower()
#                 if field_name["dataType"] == "LABELS":
#                     add_labels_to_issue(issue_number, [sent_value])
#                     continue
#                 if field_name["dataType"] == "SINGLE_SELECT":
#                     update_status_field(issue_node_id, field_name, side_infos_dict["Status"])
#                     continue
#                 if field_name["dataType"] == "ASSIGNEES":
#                     add_assignees_to_issue(issue_id, [user_id])
#                 else:
#                     query = """
#                     mutation UpdateField($input: UpdateProjectV2ItemFieldValueInput!) {
#                     updateProjectV2ItemFieldValue(input: $input) {
#                         projectV2Item {
#                         id
#                         }
#                     }
#                     }
#                     """
#                     variables = {
#                         "input": {
#                             "projectId": GITHUB_PROJECT_ID,
#                             "itemId": issue_node_id,
#                             "fieldId": field_name["id"],
#                             "value": {key: sent_value},
#                         }
#                     }
#                     run_graphql(query, variables)
#     if sprint_found is False:
#         assign_iteration_to_issue(issue_node_id, created_sprint, side_infos_dict)


def get_user_node_id(username):
    url = "https://api.github.com/graphql"
    query = """
    query GetUserId($login: String!) {
      user(login: $login) {
        id
      }
    }
    """
    variables = {"login": username}
    headers = {
        "Authorization": f"bearer {GITHUB_TOKEN}",
        "Content-Type": "application/json"
    }
    response = requests.post(url, json={
        "query": query, "variables": variables}, headers=headers)
    response.raise_for_status()
    data = response.json()
    return data["data"]["user"]["id"]


def sprint_found_in_github(items_id):
    for item in items_id["data"]["user"]["projectV2"]["fields"]["nodes"]:
        if item:
            if item["name"] == "Sprint":
                return True
    return False

# def find_issue_to_update(fetched_values):
#     final_infos = []
#     try:
#         jira_saved_infos = open("jira_save.json", "r")
#     except FileNotFoundError:
#         for issue in fetched_values["issues"]:
#             fields = issue.get("fields", {})
#             final_infos.append({"id": fields.get("id"), "existing": False})
#         return final_infos
#     jira_saved_infos_dict = json.load(jira_saved_infos)
#     i = 0
#     for issue in fetched_values["issues"]:
#         final_infos.append(is_same_infos(jira_saved_infos_dict[i], issue, fetched_values["sprints"]))
#         i += 1
#     return final_infos


def list_project_issues(project_id, per_page=50):
    query = """
    query ($project_id: ID!, $per_page: Int!, $after: String) {
  node(id: $project_id) {
    ... on ProjectV2 {
      items(first: $per_page, after: $after) {
        nodes {
          content {
            __typename
            ... on Issue {
              id
              number
              title
              body
              state
              assignees(first: 5) { nodes { login } }
              labels(first: 10) { nodes { name } }
            }
          }
          fieldValues(first: 20) {
            nodes {
              __typename
              ... on ProjectV2ItemFieldNumberValue {
                field {
                  ... on ProjectV2FieldCommon {
                    name
                  }
                }
                number
              }
              ... on ProjectV2ItemFieldIterationValue {
                field {
                  ... on ProjectV2FieldCommon {
                    name
                  }
                }
                title
                startDate
                duration
              }
              ... on ProjectV2ItemFieldSingleSelectValue {
                field {
                  ... on ProjectV2FieldCommon {
                    name
                  }
                }
                name
              }
            }
          }
        }
        pageInfo {
          hasNextPage
          endCursor
        }
      }
    }
  }
}
    """

    all_issues = []
    cursor = None

    while True:
        variables = {
            "project_id": project_id,
            "per_page": per_page,
            "after": cursor,
        }
        data = run_graphql(query, variables)

        items = data["data"]["node"]["items"]
        for item in items["nodes"]:
            content = item.get("content")
            fieldValues = item.get("fieldValues")
            if content and content["__typename"] == "Issue":
                all_issues.append((content, fieldValues))

        if items["pageInfo"]["hasNextPage"]:
            cursor = items["pageInfo"]["endCursor"]
        else:
            break

    return all_issues


# def replicate_github_issues():
    # saved_infos = []
    # jira_issues_infos = []
    # fetched_values = fetch_jira_issues()

    # list_of_infos = find_issue_to_update(fetched_values)

    # for issue in fetched_values["issues"]:
    #     fields = issue.get("fields", {})
    #     dict_of_infos = corresponding_jira_issue(fields.get("id"), list_of_infos)

    #     # Basic fields
    #     column = fields.get("status", {}).get("name", "Not set")
    #     title = fields.get("summary", "No summary provided")
    #     description = fields.get("description", "No description provided")
    #     # jira_url = f"{JIRA_BASE_URL}/browse/{issue['key']}"

    #     start_date = fields.get("customfield_10015", "Not set")
    #     due_date = fields.get("duedate", "Not set")
    #     story_points = fields.get("customfield_10016", "Not set")
    #     creation_date = fields.get("created", "Not Set").split("T")[0]
    #     assignee = fields.get("assignee", {})
    #     assignee_name = assignee.get("displayName", "Unassigned")
    #     labels = fields.get("labels", [])
    #     labels_str = ", ".join(labels) if labels else "None"
    #     # priority = fields.get("priority", {}).get("name", "Not set")

    #     # parent = fields.get("parent")
    #     # parent_info = parent.get("key") if parent else "No parent"

    #     mapped_username = json.loads(GITHUB_USERNAMES)
    #     for user in mapped_username:
    #         if user.get(assignee_name) is not None:
    #             for key, value in user.items():
    #                 assignee_name = value
    #     user_id = get_user_node_id(assignee_name)

    #     side_infos_dict = {
    #         "Start Date": start_date,
    #         "End Date": due_date,
    #         "Story point": story_points,
    #         "Sprint": fetched_values["sprints"],
    #         "Assignees": assignee_name,
    #         "Status": column,
    #         "Labels": labels_str,
    #     }

    #     # Build the GitHub issue body
    #     body = (
    #         f"{description}\n\n"
    #     )

    #     # Create label if it doesn't exist
        # create_label_if_not_exists(labels)

    #     # Create the GitHub issue
    #     if dict_of_infos["existing"] is False:
    #         issue_node_id, issue_number = create_issue_on_board(title, body)
    #         issue_id = get_github_issue(issue_number)["node_id"]
    #     else:
    #         gitub_issue_infos = get_linked_github_issue(dict_of_infos["id"])
    #         issue_node_id = gitub_issue_infos["issue_node_id"]
    #         issue_number = gitub_issue_infos["issue_number"]
    #         issue_id = gitub_issue_infos["issue_id"]

        # items_id = get_project_details()
    #     # # print(items_id)

    #     existing_sprint, fields_name = sprint_field_is_already_existing(
    #         items_id, fetched_values["sprints"], creation_date)
    #     created_sprint = {}
    #     if existing_sprint is False:
    #         created_sprint = create_iteration_field(fields_name, creation_date)

    #     update_infos(issue_node_id,
    #                  side_infos_dict, items_id,
    #                  user_id, issue_id, issue_number, created_sprint)
    #     jira_infos = {
    #         "id": fields.get("id"),
    #         "infos": {
    #             "title": title,
    #             "description": description,
    #             "side_infos": side_infos_dict,
    #             "created_sprint": created_sprint,
    #             },
    #     }
    #     jira_issues_infos.append(jira_infos)

    #     infos = {
    #         "title": title,
    #         "linked_jira_issue_number": fields.get("id"),
    #         "description": description,
    #         "issue_node_id": issue_node_id,
    #         "issue_number": issue_number,
    #         "issue_id": issue_id,
    #         "user_id": user_id,
    #         "side_infos": side_infos_dict,
    #         "created_sprint": created_sprint,
    #     }

    #     saved_infos.append(infos)

    # with open("jira_save.json", "w") as file:
    #     json.dump(jira_issues_infos, file)
    # with open("github_save.json", "w") as file:
    #     json.dump(saved_infos, file)

headers = {
    "Authorization": f"bearer {GITHUB_TOKEN}",
    "Content-Type": "application/json",
    # "Accept": "application/vnd.github.starfox-preview+json",
    "Accept": "application/vnd.github+json"
}


# def get_github_issue(issue_number):
#     url = f"https://api.github.com/repos/{GITHUB_PROJECT_OWNER}/{SRC_REPO}/issues/{issue_number}"
#     headers = {
#         "Authorization": f"Bearer {GITHUB_TOKEN}",
#         "Accept": "application/vnd.github.v3+json"
#     }
#     response = requests.get(url, headers=headers)
#     if response.status_code == 200:
#         return response.json()
#     else:
#         raise Exception(f"Failed to fetch issue: {response.status_code}, {response.text}")


def add_to_project(project_id, issue_id):
    mutation = """
    mutation ($input: AddProjectV2ItemByIdInput!) {
      addProjectV2ItemById(input: $input) { item { id } }
    }
    """
    run_graphql(mutation, {"input": {
        "projectId": project_id, "contentId": issue_id}})


def add_labels(owner, repo, issue_number, labels):
    url = f"https://api.github.com/repos/{owner}/{repo}/issues/{issue_number}/labels"
    requests.post(url, headers=headers, json={"labels": labels})


def add_assignees(owner, repo, issue_number, assignees):
    url = f"https://api.github.com/repos/{owner}/{repo}/issues/{issue_number}/assignees"
    requests.post(url, headers=headers, json={"assignees": assignees})


def add_story_points(issue_node_id, project_id, field_id, points):
    query = """
    mutation UpdateProjectV2ItemFieldValue($input: UpdateProjectV2ItemFieldValueInput!) {
      updateProjectV2ItemFieldValue(input: $input) {
        projectV2Item {
          id
        }
      }
    }
    """
    variables = {
        "input": {
            "projectId": project_id,
            "itemId": issue_node_id,
            "fieldId": field_id,
            "value": {"number": points},
        }
    }
    run_graphql(query, variables)


def add_status(issue_node_id, project_id, field_id, status_name):
    option_id = resolve_single_select_option_id(
        project_id, field_id, status_name)
    query = """
    mutation UpdateStatusField($input: UpdateProjectV2ItemFieldValueInput!) {
      updateProjectV2ItemFieldValue(input: $input) {
        projectV2Item {
          id
        }
      }
    }
    """
    variables = {
        "input": {
            "projectId": project_id,
            "itemId": issue_node_id,
            "fieldId": field_id,
            "value": {"singleSelectOptionId": option_id},
        }
    }
    run_graphql(query, variables)


def add_sprint(issue_node_id, project_id, field_id, sprint_name):
    option_id = resolve_iteration_id(project_id, field_id, sprint_name)
    query = """
    mutation UpdateIterationField($input: UpdateProjectV2ItemFieldValueInput!) {
      updateProjectV2ItemFieldValue(input: $input) {
        projectV2Item {
          id
        }
      }
    }
    """
    variables = {
        "input": {
            "projectId": project_id,
            "itemId": issue_node_id,
            "fieldId": field_id,
            "value": {"iterationId": option_id},
        }
    }
    run_graphql(query, variables)


def get_single_select_options(project_id, field_id):
    query = """
    query ($project_id: ID!) {
      node(id: $project_id) {
        ... on ProjectV2 {
          fields(first: 50) {
            nodes {
              ... on ProjectV2SingleSelectField {
                id
                name
                options {
                  id
                  name
                }
              }
            }
          }
        }
      }
    }
    """
    variables = {"project_id": project_id}
    data = run_graphql(query, variables)

    fields = data["data"]["node"]["fields"]["nodes"]

    for f in fields:
        if "id" in f and f["id"] == field_id:
            return f.get("options", [])

    print(f"No single-select field found with id {field_id}")
    return []


def resolve_single_select_option_id(project_id, field_id, option_name):
    options = get_single_select_options(project_id, field_id)
    for opt in options:
        if opt["name"] == option_name:
            return opt["id"]
    return None


def get_iteration_options(project_id, field_id):
    query = """
    query ($project_id: ID!) {
      node(id: $project_id) {
        ... on ProjectV2 {
          fields(first: 50) {
            nodes {
              ... on ProjectV2IterationField {
                id
                name
                configuration {
                  iterations {
                    id
                    title
                    startDate
                    duration
                  }
                  completedIterations {
                    id
                    title
                    startDate
                    duration
                  }
                }
              }
            }
          }
        }
      }
    }
    """

    variables = {"project_id": project_id}
    data = run_graphql(query, variables)
    fields = data["data"]["node"]["fields"]["nodes"]

    for f in fields:
        if f.get("id") == field_id:
            config = f["configuration"]
            return (config.get("iterations", []) +
                    config.get("completedIterations", []))

    print(f"No iteration field found with id {field_id}")
    return []


def resolve_iteration_id(project_id, field_id, iteration_title):
    iterations = get_iteration_options(project_id, field_id)

    for it in iterations:
        if it["title"] == iteration_title:
            return it["id"]

    print(f"No iteration found with title '{iteration_title}' in field {field_id}")
    return None


def get_current_field_id(field_name):
    query = """
    query ($project_id: ID!) {
      node(id: $project_id) {
        ... on ProjectV2 {
          fields(first: 100) {
            nodes {
              ... on ProjectV2FieldCommon {
                id
                name
              }
            }
          }
        }
      }
    }
    """
    variables = {"project_id": DST_PROJECT_ID}
    data = run_graphql(query, variables)
    fields = data["data"]["node"]["fields"]["nodes"]
    for field in fields:
        if field["name"] == field_name:
            return field["id"]
    return None


def get_issue_comments(owner, repo, issue_number):
    url = f"https://api.github.com/repos/{owner}/{repo}/issues/{issue_number}/comments"
    headers = {"Authorization": f"Bearer {GITHUB_TOKEN}"}
    response = requests.get(url, headers=headers)
    response.raise_for_status()
    return response.json()


def transfer_issues():
    saved_infos = []
    print(f"Fetching issues from {SRC_OWNER}/{SRC_REPO}...")
    source_issues = list_project_issues(GITHUB_PROJECT_ID)
    print(f"Found {len(source_issues)} issues")

    target_repo_id = get_repository_id(DST_OWNER, DST_REPO)
    print(target_repo_id)
    for issue in source_issues:
        issue_base_values = issue[0]
        issue_fields_values = issue[1]
        print(issue_base_values)
        print(issue_fields_values)
        print(f"Transferring: #{issue_base_values['number']} {issue_base_values['title']}")
        issue_comment = get_issue_comments(
            SRC_OWNER, SRC_REPO, issue_base_values['number'])
        new_body = issue_base_values["body"]
        for comment in issue_comment:
            new_body += f"\n\n---\n### Imported comment by **{comment['user']['login']}**\n\n{comment['body']}"

        issue_node_id, issue_number, issue_id = create_issue_on_board(
            issue_base_values["title"], new_body, target_repo_id
            )
        print(f"Issue created: #{issue_number}")
        # Add labels
        labels = [label["name"] for label in issue_base_values["labels"]["nodes"]]
        if labels:
            add_labels(DST_OWNER, DST_REPO, issue_number, labels)

        # Add assignees
        assignees = [assignee["login"] for assignee in issue_base_values["assignees"]["nodes"]]
        if assignees:
            add_assignees(DST_OWNER, DST_REPO, issue_number, assignees)
        story_points = [s["number"] for s in issue_fields_values["nodes"] if ("field" in s and s["field"]["name"] == "Story points")]
        story_points_field_id = get_current_field_id("Story points")
        if story_points:
            add_story_points(issue_node_id, DST_PROJECT_ID,  story_points_field_id, story_points[0])

        sprint = [s for s in issue_fields_values["nodes"] if ("field" in s and s["field"]["name"] == "Sprint")]
        sprint_field_id = get_current_field_id("Sprint")
        if sprint:
            add_sprint(issue_node_id, DST_PROJECT_ID, sprint_field_id, sprint[0]["title"])

        status = [s["name"] for s in issue_fields_values["nodes"] if ("field" in s and s["field"]["name"] == "Status")]
        status_field_id = get_current_field_id("Status")
        if status:
            add_status(issue_node_id, DST_PROJECT_ID, status_field_id, status[0])
        side_infos_dict = {
            "Story point": story_points,
            "Sprint": sprint,
            "Assignees": assignees,
            "Status": status,
            "Labels": labels,
        }

        # issue_id = get_github_issue(issue_number)
        # print(f"Issue ID: {issue_id}")
        infos = {
            "title": issue_base_values["title"],
            "description": new_body,
            "issue_node_id": issue_node_id,
            "issue_number": issue_number,
            "issue_id": issue_id,
            "user_id": get_user_node_id(assignees[0]) if assignees else None,
            "side_infos": side_infos_dict,
            }
        saved_infos.append(infos)
    # If the save file does not exist, create it
    # if not os.path.exists(f"history/{DST_REPO}_save.json"):
        # os.makedirs("history", exist_ok=True)

    with open(f"history/{DST_REPO}_save.json", "w") as file:
        json.dump(saved_infos, file, indent=4)


def delete_all_project_issues(project_id, per_page=50):
    query_list = """
    query ($project_id: ID!, $per_page: Int!, $after: String) {
      node(id: $project_id) {
        ... on ProjectV2 {
          items(first: $per_page, after: $after) {
            nodes {
              id
              content {
                __typename
                ... on Issue {
                  id
                  number
                }
              }
            }
            pageInfo {
              hasNextPage
              endCursor
            }
          }
        }
      }
    }
    """

    items_to_delete = []
    cursor = None
    while True:
        variables = {"project_id": project_id, "per_page": per_page, "after": cursor}
        data = run_graphql(query_list, variables)
        print(data)
        items = data["data"]["node"]["items"]

        for node in items["nodes"]:
            content = node.get("content")
            if content and content.get("__typename") == "Issue":
                items_to_delete.append(node["id"])

        if items["pageInfo"]["hasNextPage"]:
            cursor = items["pageInfo"]["endCursor"]
        else:
            break

    print(f"Found {len(items_to_delete)} issue items to delete from project {project_id}")

    mutation_delete = """
    mutation ($input: DeleteProjectV2ItemInput!) {
      deleteProjectV2Item(input: $input) {
        deletedItemId
      }
    }
    """

    for item_id in items_to_delete:
        vars_delete = {"input": {"projectId": project_id, "itemId": item_id}}
        result = run_graphql(mutation_delete, vars_delete)
        deleted = result.get("data", {}).get("deleteProjectV2Item", {}).get("deletedItemId")
        print(f"Deleted item {item_id} -> {deleted}")

    print("All issue items removed.")


if __name__ == "__main__":
    # transfer_issues()
    # delete_all_project_issues(DST_PROJECT_ID)
    # print(history_dir)
    while (True):
        pass
