FROM python:alpine3.20

WORKDIR /app

RUN python -m venv .venv
RUN source .venv/bin/activate

COPY ./requirements.txt /app/requirements.txt

RUN pip install -r /app/requirements.txt

COPY ./.env /app/.env
COPY ./history /app/history
COPY ./replicateGithubIssues.py /app/replicateGithubIssues.py

CMD ["/app/replicateGithubIssues.py", "/app/.env"]