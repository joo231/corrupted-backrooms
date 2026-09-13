# Corrupted Backrooms

Игра для [itch.io](https://itch.io): **Unreal Engine 4.27.2**, C++ + Blueprints.
Исходники на GitHub, готовая сборка — на itch.io.

Требования: **UE 4.27.2**, Git, [Git LFS](https://git-lfs.com), Visual Studio 2022 с workload *Game development with C++* и toolset **MSVC v142**.

## Другу: как подключиться

1. Поставь тот же **Unreal Engine 4.27.2** из Epic Games Launcher.
2. Поставь Git и Git LFS, затем в PowerShell: `git lfs install`
3. Склонируй репозиторий (ссылку пришлёт владелец после приглашения Collaborator):

```powershell
git clone https://github.com/joo231/corrupted-backrooms.git
cd corrupted-backrooms
```

4. Открой `CorruptedBackrooms.uproject` двойным щелчком (движок сам соберёт C++ модуль).
5. Если Visual Studio ругается на компилятор — в Visual Studio Installer добавь **MSVC v142 (VS 2019 C++ x64/x86 build tools)**.

## Правила, без которых ломаются `.uasset`

- Перед работой всегда `git pull`.
- **Не правьте один и тот же уровень / Blueprint / ассет одновременно.**
- Перед коммитом спорных ассетов закройте редактор (или коммитьте через Source Control в UE).
- `main` — стабильная ветка. Фичи — в `feature/имя`, вливать через Pull Request.
- Не коммитьте `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`, `Build/` — они в `.gitignore`.
- Крупные видео и текстуры лучше не класть в git: у GitHub Free лимит **1 GB LFS**.

## Как залить игру на itch.io

GitHub — это исходники, не страница для игроков.

1. Зарегистрируйтесь на [itch.io](https://itch.io) и создайте проект (Windows, downloadable).
2. В редакторе: **File → Package Project → Windows (64-bit)**.
3. Заархивируйте папку сборки в zip и загрузите её на страницу itch.
4. Сборку в git не кладите. По желанию zip можно положить в GitHub Releases.
