// The Go side, through the generated Wails bindings.
import { Events } from "@wailsio/runtime";
import * as Service from "../../bindings/github.com/KayraBulbul/sysroad/internal/gui/service";
import type {
  Snapshot,
  LessonDetail,
  File,
  FileEvent,
  ResultEvent,
  Theme,
  PhaseDTO,
  ChapterDTO,
  LessonRow,
  UpdateInfo,
  ProjectInfo,
  CaughtUp,
} from "../../bindings/github.com/KayraBulbul/sysroad/internal/gui/models";
import { ProjectState } from "../../bindings/github.com/KayraBulbul/sysroad/internal/gui/models";
import type { Result } from "../../bindings/github.com/KayraBulbul/sysroad/internal/course/models";
import { Mode } from "../../bindings/github.com/KayraBulbul/sysroad/internal/course/models";

export { Service, Mode, ProjectState };
export type {
  Snapshot,
  LessonDetail,
  File,
  FileEvent,
  ResultEvent,
  Theme,
  PhaseDTO,
  ChapterDTO,
  LessonRow,
  UpdateInfo,
  ProjectInfo,
  CaughtUp,
  Result,
};

// tilde shortens a path in your home folder to ~/…, for display.
export function tilde(path: string): string {
  return path.replace(/^\/(home|Users)\/[^/]+/, "~");
}

export function on<T>(name: string, fn: (data: T) => void): () => void {
  return Events.On(name, (ev: { data: T }) => fn(ev.data));
}

// errText turns a rejected binding call into a message.
export function errText(e: unknown): string {
  if (e && typeof e === "object" && "message" in e) return String((e as { message: unknown }).message);
  return String(e);
}
